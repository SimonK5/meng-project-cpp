#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geometry.h"
#include "node.h"

class RTree {
 private:
  std::shared_ptr<Rect> rect;
  int max_per_level;

  void find_intersections(std::shared_ptr<Node> n,
                          const std::shared_ptr<Rect> area,
                          std::vector<std::shared_ptr<Node>> res) {
    if (n->level == 0) {
      for (auto it = n->children.begin(); it != n->children.end(); ++it) {
        if (area->intersects((*it)->obj->rect)) {
          res.push_back((*it)->obj);
        }
      }
    } else {
      for (auto it = n->children.begin(); it != n->children.end(); ++it) {
        if (area->contains((*it)->rect) || area->intersects((*it)->rect)) {
          find_intersections((*it), area, res);
        }
      }
    }
  }

  std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> RTree::pick_seeds(
      std::shared_ptr<Node> n) {
    std::shared_ptr<Node> max_low_side_entry_x =
        std::make_shared<Node>(nullptr);
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      if (max_low_side_entry_x.get() == nullptr ||
          (*it)->rect->x1 > max_low_side_entry_x->rect->x1 &&
              (*it).get() != max_low_side_entry_x.get()) {
        max_low_side_entry_x = *it;
      }
    }

    std::shared_ptr<Node> min_high_side_entry_x =
        std::make_shared<Node>(nullptr);
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      if (min_high_side_entry_x.get() == nullptr ||
          (*it)->rect->x2 < min_high_side_entry_x->rect->x2 &&
              (*it).get() != min_high_side_entry_x.get()) {
        min_high_side_entry_x = *it;
      }
    }

    std::shared_ptr<Node> max_low_side_entry_y =
        std::make_shared<Node>(nullptr);
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      if (max_low_side_entry_y.get() == nullptr ||
          (*it)->rect->y1 > max_low_side_entry_x->rect->y1 &&
              (*it).get() != max_low_side_entry_y.get()) {
        max_low_side_entry_y = *it;
      }
    }
    std::shared_ptr<Node> min_high_side_entry_y =
        std::make_shared<Node>(nullptr);
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      if (min_high_side_entry_y.get() == nullptr ||
          (*it)->rect->y2 < min_high_side_entry_x->rect->y2 &&
              (*it).get() != min_high_side_entry_y.get()) {
        min_high_side_entry_y = *it;
      }
    }

    double separation_x =
        (min_high_side_entry_x->rect->x2 - max_low_side_entry_x->rect->x1) /
        (n->rect->x2 - n->rect->x1);
    double separation_y =
        (min_high_side_entry_y->rect->y2 - max_low_side_entry_y->rect->y1) /
        (n->rect->y2 - n->rect->y1);

    std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> seeds;
    if (separation_y > separation_x) {
      seeds.first = max_low_side_entry_y;
      seeds.second = min_high_side_entry_y;
    } else {
      seeds.first = max_low_side_entry_x;
      seeds.second = min_high_side_entry_x;
    }

    return seeds;
  }

  std::shared_ptr<Node> pick_next(
      std::vector<std::shared_ptr<Node>> children,
      std::unordered_set<std::shared_ptr<Node>> added,
      std::shared_ptr<Rect> bbox_1, std::shared_ptr<Rect> bbox_2) {
    int max_diff = 0;
    std::shared_ptr<Node> max_child = std::make_shared<Node>(nullptr);
    for (auto it = children.begin(); it != children.end(); ++it) {
      std::shared_ptr<Node> c = (*it);
      if (added.find(c) == added.end()) {
        continue;
      }

      std::shared_ptr<Rect> enlarged_1 = bbox_1->enlargeToContain(c->rect);
      std::shared_ptr<Rect> enlarged_2 = bbox_2->enlargeToContain(c->rect);
      float diff = std::abs(enlarged_1->area() - enlarged_2->area());
      if (diff > max_diff) {
        max_child = c;
        max_diff = diff;
      }
    }

    return max_child;
  }

  std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> RTree::linear_split(
      std::shared_ptr<Node> n) {}

  std::shared_ptr<Rect> get_enlarged_bb(std::shared_ptr<Node> n,
                                        std::shared_ptr<Node> best_child,
                                        std::shared_ptr<Rect> container_rect) {
    int min_area_increase = INT32_MAX;
    std::shared_ptr<Rect> best_bb = std::shared_ptr<Rect>(nullptr);
    for (auto it = n->children.begin(); it != n->children.end(); ++it) {
      std::shared_ptr<Rect> r = (*it)->rect;
      std::shared_ptr<Rect> enlarged_bb = r->enlargeToContain(container_rect);
      int area_increase = (*enlarged_bb).area() - (*r).area();
      if (area_increase < min_area_increase) {
        min_area_increase = area_increase;
        best_bb = enlarged_bb;
        best_child = (*it);
      }
    }

    return best_bb;
  }

  void insert_helper(std::shared_ptr<Node> n, std::shared_ptr<Node> obj,
                     int buffer) {
    std::shared_ptr<Rect> container_rect =
        std::make_shared<Rect>(obj->rect->x1 - buffer, obj->rect->y1 - buffer,
                               obj->rect->x2 + buffer, obj->rect->y2 + buffer);

    if (n->level > 0) {
      std::shared_ptr<Node> best_child = std::shared_ptr<Node>(nullptr);
      bool fits_in_child = false;
      for (auto it = n->children.begin(); it != n->children.end(); ++it) {
        std::shared_ptr<Rect> r = (*it)->rect;
        if ((*r).contains(container_rect)) {
          insert_helper(obj, *it, buffer);
          fits_in_child = true;
          best_child = (*it);
          break;
        }
      }

      // If no child node can fit this object, expand one of them to fit it
      if (!fits_in_child) {
        best_child->rect = get_enlarged_bb(n, best_child, container_rect);
        insert_helper(obj, best_child, buffer);
      }

      if ((*best_child).children.size() > max_per_level) {
        // TODO: Split node
      }
    } else {
      std::vector<std::shared_ptr<Node>> emptyVector;
      std::shared_ptr<Node> container_node =
          std::make_shared<Node>(container_rect, emptyVector, -1);
      n->children.push_back(container_node);
      // TODO: Update particle_to_bbox
    }
  }

 public:
  std::shared_ptr<Node> root;
  // std::unordered_map<Node, Rect> particle_to_bbox;
  RTree(int x1, int y1, int x2, int y2, int max_per_level = 5)
      : rect(new Rect(x1, y1, x2, y2)), root(new Node(x1, y1, x2, y2, {}, 0)){};

  std::vector<std::shared_ptr<Node>> search(std::shared_ptr<Rect> area) {
    std::vector<std::shared_ptr<Node>> res;
    find_intersections(root, area, res);
    return res;
  }

  void insert(std::shared_ptr<Node> obj, int buffer = 20) {
    insert_helper(root, obj, buffer);
  }
};

int main() {  // Creating an instance of RTree
  RTree rTreeInstance(0, 0, 100, 100);

  // Creating nodes
  std::vector<Node> emptyVector;
  std::shared_ptr<Node> node1 =
      std::make_shared<Node>(std::make_shared<Rect>(10, 10, 30, 30));
  std::shared_ptr<Node> node2 =
      std::make_shared<Node>(std::make_shared<Rect>(40, 40, 60, 60));
  std::shared_ptr<Node> node3 =
      std::make_shared<Node>(std::make_shared<Rect>(70, 70, 90, 90));

  // Inserting nodes into the RTree instance
  rTreeInstance.insert(node1);
  rTreeInstance.insert(node2);
  rTreeInstance.insert(node3);

  // Printing the resulting root and children
  std::cout << "Root Rect: (" << rTreeInstance.root->rect->x1 << ", "
            << rTreeInstance.root->rect->y1 << "), ("
            << rTreeInstance.root->rect->x2 << ", "
            << rTreeInstance.root->rect->y2 << ")\n";

  for (const auto& child : rTreeInstance.root->children) {
    std::cout << "Child Rect: (" << child->rect->x1 << ", " << child->rect->y1
              << "), (" << child->rect->x2 << ", " << child->rect->y2 << ")\n";
  }

  return 0;
}