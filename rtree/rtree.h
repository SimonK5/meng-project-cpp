#ifndef RTREE_H
#define RTREE_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "rtreenode.h"
#include "rtreeobject.h"
#include <cugl/cugl.h>

using namespace cugl;

/**
 * Class representing an R-tree, a type of height-balanced tree used for
 * range queries. This specification also includes bounding boxes of a certain
 * buffer size around leaf nodes. Leaf nodes point to objects contained within
 * the tree.
 */
class RTree {
 private:
    /** The bounding box of the entire RTree. */
    Rect rect;

    /** The maximum amount of nodes per level. */
    unsigned int maxPerLevel;
    
    /** The minimum amount of nodes per level. */
    unsigned int minPerLevel;
    
    /** The amount of padding on each side of the bounding box of each object. */
    unsigned int bufferSize;
    
    /** Map with objects as keys and the corresponding bounding boxes as values. */
    std::unordered_map<std::shared_ptr<RTreeObject>, Rect> particleToBBox;

  /**
   * Fills a vector with objects in a subtree that intersect with a given
   * rectangular area.
   *
   * @param n The root of the subtree
   * @param area The rectangular area
   * @param res Vector containing objects that intersect the area
   */
  void find_intersections(RTreeNode &n, const Vec2 center, float radius,
                          std::vector<std::shared_ptr<RTreeObject>> &res);

  /**
   * Given a node to split, select two children of the original node to become
   * the first children of the two new nodes.
   *
   * @param n The node to split
   * @return std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> containing the first two
   * elements of the two new nodes
   */
  std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> pick_seeds(RTreeNode &n);

  /**
   * Select one remaining child of the node to be split to be added to a newly
   * split node.
   *
   * @param children Vector of children of the node to be split
   * @param added Set of children already assigned to a new node
   * @param bbox_1 The bounding box of the first new node
   * @param bbox_2 The bounding box of the second new node
   * @return RTreeNode* The next child to be added to a new node
   */
  std::shared_ptr<RTreeNode> pick_next(const std::vector<std::shared_ptr<RTreeNode>> &children,
                       const std::unordered_set<std::shared_ptr<RTreeNode>> &added,
                       const Rect &bbox_1, const Rect &bbox_2);

  /**
   * Split an overflowing node into two nodes.
   *
   * @param n The node to be split.
   * @return std::pair<RTreeNode *, RTreeNode *> The two resulting nodes from
   * the split.
   */
  std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> linear_split(RTreeNode &n);
  Rect get_enlarged_bb(RTreeNode &n, std::shared_ptr<RTreeNode> &best_child,
                        const Rect &container_rect);
  void insert_helper(RTreeNode &n, std::shared_ptr<RTreeObject> &obj);
    
    std::vector<std::shared_ptr<RTreeObject>> removeHelper(std::shared_ptr<RTreeNode> &n, std::shared_ptr<RTreeObject> &obj);

 public:
  // The root node of this RTree.
    std::shared_ptr<RTreeNode> root;

  /**
   * Resets to an empty RTree.
   */
  void clear();

  /**
   * @param x1 The x-coordinate of the lower-left corner of the root rectangle.
   * @param ->getMinY() The y-coordinate of the lower-left corner of the root
   * rectangle.
   * @param width The x-coordinate of the upper-right corner of the root
   * rectangle.
   * @param height The y-coordinate of the upper-right corner of the root
   * rectangle.
   * @param max_per_level Maximum number of children per node (default is 5).
   */
  RTree(float x1, float y1, float width, float height,
        unsigned int max_per_level = 5, unsigned int min_per_level = 2, float buffer = 20);

  /**
   * @brief Searches for objects within a given search area.
   *
   * @param area The circular search area.
   * @return A vector of shared pointers to RTreeObject instances intersecting
   * the search area.
   */
  std::vector<std::shared_ptr<RTreeObject>> search(const Vec2 center, float radius);

  /**
   * @brief Inserts an object into the R-Tree.
   *
   * @param obj Shared pointer to the RTreeObject to be inserted.
   * @param buffer Optional buffer value to expand the object's bounding box
   * (default is 20).
   */
  void insert(std::shared_ptr<RTreeObject> obj);
    
    void remove(std::shared_ptr<RTreeObject> obj);
    
    /**
     * Reconstructs this RTree using all of its existing points.
     */
    void reconstruct();
    
    void update();
    
    void draw(const std::shared_ptr<SpriteBatch>& batch);
};

#endif
