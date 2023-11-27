#ifndef RTREE_H
#define RTREE_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geometry.h"
#include "node.h"
#include "rtreeobject.h"

/**
 * Class representing an R-tree, a type of sheight-balanced tree mainly used for
 * range queries. This specification also includes bounding boxes of a certain
 * buffer size around leaf nodes. Leaf nodes point to objects contained within
 * the tree.
 */
class RTree {
 private:
  // The bounding box of the entire RTree.
  Rect *rect;

  // The maximum amount of nodes per level.
  unsigned int maxPerLevel;

  /**
   * Fills a vector with objects in a subtree that intersect with a given
   * rectangular area.
   *
   * @param n The root of the subtree
   * @param area The rectangular area
   * @param res Vector containing objects that intersect the area
   */
  void find_intersections(Node &n, const Rect &area,
                          std::vector<std::shared_ptr<RTreeObject>> &res);

  /**
   * Given a node to split, select two children of the original node to become
   * the first children of the two new nodes.
   *
   * @param n The node to split
   * @return std::pair<Node *, Node *> containing the first two elements of the
   * two new nodes
   */
  std::pair<Node *, Node *> pick_seeds(Node &n);

  /**
   * Select one remaining child of the node to be split to be added to a newly
   * split node.
   *
   * @param children Vector of children of the node to be split
   * @param added Set of children already assigned to a new node
   * @param bbox_1 The bounding box of the first new node
   * @param bbox_2 The bounding box of the second new node
   * @return Node* The next child to be added to a new node
   */
  Node *pick_next(const std::vector<Node *> &children,
                  const std::unordered_set<Node *> &added, const Rect &bbox_1,
                  const Rect &bbox_2);

  /**
   * Split an overflowing node into two nodes.
   *
   * @param n The node to be split.
   * @return std::pair<Node *, Node *> The two resulting nodes from the split.
   */
  std::pair<Node *, Node *> linear_split(Node &n);
  Rect *get_enlarged_bb(Node &n, Node *&best_child, const Rect &container_rect);
  void insert_helper(Node &n, std::shared_ptr<RTreeObject> &obj, int buffer);

 public:
  // The root node of this RTree.
  Node *root;

  /**
   * @param x1 The x-coordinate of the lower-left corner of the root rectangle.
   * @param y1 The y-coordinate of the lower-left corner of the root rectangle.
   * @param x2 The x-coordinate of the upper-right corner of the root rectangle.
   * @param y2 The y-coordinate of the upper-right corner of the root rectangle.
   * @param max_per_level Maximum number of children per node (default is 5).
   */
  RTree(int x1, int y1, int x2, int y2, unsigned int max_per_level = 5);

  /**
   * @brief Searches for objects within a given search area.
   *
   * @param area The rectangular search area.
   * @return A vector of shared pointers to RTreeObject instances intersecting
   * the search area.
   */
  std::vector<std::shared_ptr<RTreeObject>> search(Rect &area);

  /**
   * @brief Inserts an object into the R-Tree.
   *
   * @param obj Shared pointer to the RTreeObject to be inserted.
   * @param buffer Optional buffer value to expand the object's bounding box
   * (default is 20).
   */
  void insert(std::shared_ptr<RTreeObject> obj, int buffer = 20);
};

#endif
