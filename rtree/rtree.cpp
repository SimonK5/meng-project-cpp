#include "rtree.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geometry.h"
#include "node.h"
#include "rtreeobject.h"

void RTree::find_intersections(Node &n, const Rect &area,
                               std::vector<std::shared_ptr<RTreeObject>> &res) {
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

std::pair<Node *, Node *> RTree::pick_seeds(Node &n) {
  Node *max_low_side_entry_x = nullptr;
  Node *min_high_side_entry_x = nullptr;
  Node *max_low_side_entry_y = nullptr;
  Node *min_high_side_entry_y = nullptr;

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    Node *child = *it;
    if ((max_low_side_entry_x == nullptr ||
         child->rect->x1 > max_low_side_entry_x->rect->x1) &&
        child != min_high_side_entry_x) {
      max_low_side_entry_x = child;
    }
  }

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    Node *child = *it;
    if ((min_high_side_entry_x == nullptr ||
         child->rect->x2 < min_high_side_entry_x->rect->x2) &&
        child != max_low_side_entry_x) {
      min_high_side_entry_x = child;
    }
  }

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    Node *child = *it;
    if ((max_low_side_entry_y == nullptr ||
         child->rect->y1 > max_low_side_entry_y->rect->y1) &&
        child != min_high_side_entry_y) {
      max_low_side_entry_y = child;
    }
  }

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    Node *child = *it;
    if ((min_high_side_entry_y == nullptr ||
         child->rect->y2 < min_high_side_entry_y->rect->y2) &&
        child != max_low_side_entry_y) {
      min_high_side_entry_y = child;
    }
  }

  double separation_x = (double)(min_high_side_entry_x->rect->x2 -
                                 max_low_side_entry_x->rect->x1) /
                        (n.rect->x2 - n.rect->x1);
  double separation_y = (double)(min_high_side_entry_y->rect->y2 -
                                 max_low_side_entry_y->rect->y1) /
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

Node *RTree::pick_next(const std::vector<Node *> &children,
                       const std::unordered_set<Node *> &added,
                       const Rect &bbox_1, const Rect &bbox_2) {
  int max_diff = 0;
  Node *max_child = nullptr;
  for (auto it = children.begin(); it != children.end(); ++it) {
    Node *child = *it;
    if (added.find(child) != added.end()) {
      continue;
    }

    Rect enlarged_1 = bbox_1.enlargeToContain(*(child->rect));
    Rect enlarged_2 = bbox_2.enlargeToContain(*(child->rect));
    float diff = std::abs(enlarged_1.area() - enlarged_2.area());

    if (max_child == nullptr || diff > max_diff) {
      max_child = child;
      max_diff = diff;
    }
  }

  return max_child;
}

std::pair<Node *, Node *> RTree::linear_split(Node &n) {
  std::pair<Node *, Node *> seeds = pick_seeds(n);

  Node *c1 = seeds.first;
  Node *c2 = seeds.second;

  std::unordered_set<Node *> added;
  Node *node1 = new Node(c1->rect, {}, n.level);
  Node *node2 = new Node(c2->rect, {}, n.level);

  node1->children.push_back(c1);
  node2->children.push_back(c2);
  added.insert(c2);
  added.insert(c1);
  while (added.size() < n.children.size()) {
    Node *nextNode =
        pick_next(n.children, added, *(node1->rect), *(node2->rect));

    added.insert(nextNode);
    Rect enlarged1 = node1->rect->enlargeToContain(*(nextNode->rect));
    Rect enlarged2 = node2->rect->enlargeToContain(*(nextNode->rect));
    if (enlarged1.area() < enlarged2.area()) {
      node1->children.push_back(nextNode);
      node1->rect = new Rect(enlarged1);
    } else {
      node2->children.push_back(nextNode);
      node2->rect = new Rect(enlarged2);
    }
  }
  return std::make_pair(node1, node2);
}

Rect *RTree::get_enlarged_bb(Node &n, Node *&best_child,
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

void RTree::insert_helper(Node &n, std::shared_ptr<RTreeObject> &obj,
                          int buffer) {
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
    if ((*best_child).children.size() > maxPerLevel) {
      std::pair<Node *, Node *> nodes = linear_split(*best_child);
      n.deleteChild(*best_child);
      n.addChild(*(nodes.first));
      n.addChild(*(nodes.second));
    }
  } else {
    std::vector<Node *> emptyVector;
    Node *container_node = new Node(container_rect, emptyVector, -1);
    container_node->obj = obj;
    n.children.push_back(container_node);
    // TODO: Update particle_to_bbox
  }
}

// std::unordered_map<Node, Rect> particle_to_bbox;
RTree::RTree(int x1, int y1, int x2, int y2, unsigned int max_per_level)
    : rect(new Rect(x1, y1, x2, y2)),
      maxPerLevel(max_per_level),
      root(new Node(x1, y1, x2, y2, {}, 0)){};

std::vector<std::shared_ptr<RTreeObject>> RTree::search(Rect &area) {
  std::vector<std::shared_ptr<RTreeObject>> res;
  find_intersections(*root, area, res);
  return res;
}

void RTree::insert(std::shared_ptr<RTreeObject> obj, int buffer) {
  insert_helper(*root, obj, buffer);
  if (root->children.size() > maxPerLevel) {
    Node *newRoot = new Node(root->rect, {}, root->level + 1);
    std::pair<Node *, Node *> nodes = linear_split(*root);
    newRoot->addChild(*(nodes.first));
    newRoot->addChild(*(nodes.second));
    root = newRoot;
  }
}

int main() {
  RTree rTreeInstance(0, 0, 100, 100, 5);

  int numberOfObjects = 100;
  int w = 5;
  int h = 5;

  std::vector<std::shared_ptr<RTreeObject>> objects;

  for (int i = 0; i < numberOfObjects; ++i) {
    int x1 = rand() % 100;
    int y1 = rand() % 100;
    int x2 = x1 + w;
    int y2 = y1 + h;

    rTreeInstance.insert(std::make_shared<RTreeObject>(x1, y1, x2, y2));
  }

  std::string treeStr = rTreeInstance.root->print(rTreeInstance.root->level);
  std::cout << treeStr << std::endl;
  return 0;
}