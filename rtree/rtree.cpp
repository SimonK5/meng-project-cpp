#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geometry.h"
#include "node.h"
#include "rtreeobject.h"

class RTree {
 private:
  Rect *rect;
  unsigned int max_per_level;

  void find_intersections(Node &n, const Rect &area, std::vector<Node *> &res) {
    if (n.level == 0) {
      for (auto &child : n.children) {
        if (area.intersects(*child->obj->rect)) {
          res.push_back(child->obj);
        }
      }
    } else {
      for (auto &child : n.children) {
        if (area.contains(*(child->rect)) || area.intersects(*(child->rect))) {
          find_intersections(*child, area, res);
        }
      }
    }
  }

  std::pair<Node *, Node *> pick_seeds(Node &n) {
    Node *max_low_side_entry_x = nullptr;
    Node *min_high_side_entry_x = nullptr;
    Node *max_low_side_entry_y = nullptr;
    Node *min_high_side_entry_y = nullptr;

    for (auto &child : n.children) {
      if (max_low_side_entry_x == nullptr ||
          (child->rect->x1 > max_low_side_entry_x->rect->x1 &&
           child != max_low_side_entry_x)) {
        max_low_side_entry_x = child;
      }

      if (min_high_side_entry_x == nullptr ||
          (child->rect->x2 < min_high_side_entry_x->rect->x2 &&
           child != min_high_side_entry_x)) {
        min_high_side_entry_x = child;
      }

      if (max_low_side_entry_y == nullptr ||
          (child->rect->y1 > max_low_side_entry_y->rect->y1 &&
           child != max_low_side_entry_y)) {
        max_low_side_entry_y = child;
      }

      if (min_high_side_entry_y == nullptr ||
          (child->rect->y2 < min_high_side_entry_y->rect->y2 &&
           child != min_high_side_entry_y)) {
        min_high_side_entry_y = child;
      }
    }

    double separation_x =
        (min_high_side_entry_x->rect->x2 - max_low_side_entry_x->rect->x1) /
        (n.rect->x2 - n.rect->x1);
    double separation_y =
        (min_high_side_entry_y->rect->y2 - max_low_side_entry_y->rect->y1) /
        (n.rect->y2 - n.rect->y1);

    std::pair<Node *, Node *> seeds;
    if (separation_y > separation_x) {
      seeds.first = max_low_side_entry_y;
      seeds.second = min_high_side_entry_y;
    } else {
      seeds.first = max_low_side_entry_x;
      seeds.second = min_high_side_entry_x;
    }

    return seeds;
  }

  Node *pick_next(const std::vector<Node *> &children,
                  const std::unordered_set<Node *> &added, const Rect &bbox_1,
                  const Rect &bbox_2) {
    int max_diff = 0;
    Node *max_child = nullptr;

    for (auto &child : children) {
      if (added.find(child) == added.end()) {
        continue;
      }

      Rect enlarged_1 = bbox_1.enlargeToContain(*(child->rect));
      Rect enlarged_2 = bbox_2.enlargeToContain(*(child->rect));
      float diff = std::abs(enlarged_1.area() - enlarged_2.area());

      if (diff > max_diff) {
        max_child = child;
        max_diff = diff;
      }
    }

    return max_child;
  }

  std::pair<Node *, Node *> linear_split(Node &n) {}

  Rect *get_enlarged_bb(Node &n, Node *&best_child,
                        const Rect &container_rect) {
    int min_area_increase = INT32_MAX;
    Rect *best_bb = nullptr;

    for (auto &child : n.children) {
      Rect *r = child->rect;
      Rect enlarged_bb = r->enlargeToContain(container_rect);
      int area_increase = enlarged_bb.area() - r->area();

      if (area_increase < min_area_increase) {
        min_area_increase = area_increase;
        best_bb = new Rect(enlarged_bb);
        best_child = child;
      }
    }

    return best_bb;
  }

  void insert_helper(Node &n, std::shared_ptr<RTreeObject> &obj, int buffer) {
    Rect *container_rect =
        new Rect(obj->rect->x1 - buffer, obj->rect->y1 - buffer,
                 obj->rect->x2 + buffer, obj->rect->y2 + buffer);

    if (n.level > 0) {
      Node *best_child = nullptr;
      bool fits_in_child = false;
      for (auto it = n.children.begin(); it != n.children.end(); ++it) {
        Rect *r = (*it)->rect;
        if ((*r).contains(*container_rect)) {
          insert_helper(*(*it), obj, buffer);
          fits_in_child = true;
          best_child = (*it);
          break;
        }
      }

      // If no child node can fit this object, expand one of them to fit it
      if (!fits_in_child) {
        best_child->rect = get_enlarged_bb(n, best_child, *container_rect);
        insert_helper(*best_child, obj, buffer);
      }

      if ((*best_child).children.size() > max_per_level) {
        // TODO: Split node
      }
    } else {
      std::vector<Node *> emptyVector;
      Node *container_node = new Node(container_rect, emptyVector, -1);
      n.children.push_back(container_node);
      // TODO: Update particle_to_bbox
    }
  }

 public:
  Node *root;
  // std::unordered_map<Node, Rect> particle_to_bbox;
  RTree(int x1, int y1, int x2, int y2, unsigned int max_per_level = 2)
      : rect(new Rect(x1, y1, x2, y2)), root(new Node(x1, y1, x2, y2, {}, 0)){};

  std::vector<Node *> search(Rect &area) {
    std::vector<Node *> res;
    find_intersections(*root, area, res);
    return res;
  }

  void insert(std::shared_ptr<RTreeObject> obj, int buffer = 20) {
    insert_helper(*root, obj, buffer);
  }
};

int main() {  // Creating an instance of RTree
  RTree rTreeInstance(0, 0, 100, 100);

  std::shared_ptr<RTreeObject> o1 =
      std::make_shared<RTreeObject>(10, 10, 30, 30);
  std::shared_ptr<RTreeObject> o2 =
      std::make_shared<RTreeObject>(40, 40, 60, 60);
  std::shared_ptr<RTreeObject> o3 =
      std::make_shared<RTreeObject>(70, 70, 90, 90);

  // Inserting nodes into the RTree instance
  rTreeInstance.insert(o1);
  rTreeInstance.insert(o2);
  rTreeInstance.insert(o3);

  // Printing the resulting root and children
  std::cout << "Root Rect: (" << rTreeInstance.root->rect->x1 << ", "
            << rTreeInstance.root->rect->y1 << "), ("
            << rTreeInstance.root->rect->x2 << ", "
            << rTreeInstance.root->rect->y2 << ")\n";

  for (const auto &child : rTreeInstance.root->children) {
    std::cout << "Child Rect: (" << child->rect->x1 << ", " << child->rect->y1
              << "), (" << child->rect->x2 << ", " << child->rect->y2 << ")\n";
  }

  return 0;
}