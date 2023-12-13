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
    std::unordered_map<std::shared_ptr<RTreeObject>, Rect> objectToBBox;

    /**
     * Fills a vector with objects in a subtree that intersect with a given
     * circular area.
     *
     * @param n The root of the subtree.
     * @param center The center of the circle.
     * @param radius The radius of the circle.
     * @param res Vector containing objects that intersect the area.
     */
    void findIntersections(RTreeNode &n,
        const Vec2 center, float radius,
            std::vector<std::shared_ptr<RTreeObject>> &res);

    /**
     * Given a node to split, selects two children of the original node to become
     * the first children of the two new nodes.
     *
     * @param n The node to split
     * @return Pair of RTreeNode shared pointers containing the first two elements of the two new nodes
     */
    std::pair <std::shared_ptr<RTreeNode>,
    std::shared_ptr<RTreeNode>> pickSeeds(RTreeNode &n);

    /**
     * Selects one remaining child of the node to be split to be added to a newly
     * split node.
     *
     * @param children Vector of children of the node to be split
     * @param added Set of children already assigned to a new node
     * @param bbox_1 The bounding box of the first new node
     * @param bbox_2 The bounding box of the second new node
     * @return RTreeNode* The next child to be added to a new node
     */
    std::shared_ptr<RTreeNode> pickNext(const std::vector<std::shared_ptr<RTreeNode>> &children,
        const std::unordered_set <std::shared_ptr<RTreeNode>> &added, const Rect &bbox_1, const Rect &bbox_2);

    /**
     * Splits an overflowing node into two nodes.
     *
     * @param n The node to be split.
     * @return The two resulting nodes from the split.
     */
    std::pair <std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> linearSplit(RTreeNode &n);
    
    /**
     * Given a rectangle, determine the child bounding box such that the union of the new rectangle and
     * child bounding box is minimal.
     *
     * @param n The parent of the candidate child nodes to be checked.
     * @param containerRect The bounding box of the object to be inserted
     * @return The node that can expand to fit containerRect with minimal area increase.
     */
    std::shared_ptr<RTreeNode> findBestBB(RTreeNode &n, const Rect &containerRect);
    
    /**
     * Inserts an object into a node.
     *
     * @param n The node into which the object will be inserted.
     * @param obj The object to insert.
     */
    void insertHelper(RTreeNode &n, std::shared_ptr<RTreeObject> &obj);

    /**
     * Searches for an object in a given node, and if it is found, removes it.
     *
     * If removing the object causes the node to have too few children, the
     * remaining children are removed and returned.
     *
     * @param n The node to search.
     * @param obj The object to be removed.
     * @return A list of objects to reinsert into the tree.
     */
    std::vector<std::shared_ptr<RTreeObject>> removeHelper(std::shared_ptr<RTreeNode> &n, std::shared_ptr<RTreeObject> &obj);
    
    /**
     * Partition a list of child nodes into a certain amount of new parent nodes.
     *
     * @param nodes Vector of nodes to be partitioned
     * @return Vector of the new children nodes
     */
    std::vector<std::shared_ptr<RTreeNode>> strSplit(std::vector<std::shared_ptr<RTreeNode>> &nodes, int level);
    
    /**
     * Build an R-Tree from the bottom up using a list of nodes.
     *
     * Uses the Sort-Tile-Recursive (STR) algorithm to build an rtree using a bulk
     * insertion. This builds trees faster than inserting objects one-by-one and
     * results in less overlaps between subtrees.
     *
     * Precondition: nodes is non-empty.
     *
     * @param nodes The list of nodes to be bulk inserted into the RTree.
     * @return The root node of the new RTree.
     */
    std::shared_ptr<RTreeNode> sortTileRecursive(std::vector<std::shared_ptr<RTreeNode>> &nodes);

public:
    /** The root node of this RTree. */
    std::shared_ptr<RTreeNode> root;

    /**
     * Resets to an empty RTree.
     */
    void clear();

    /**
     * Creates an RTree.
     *
     * @param x The x-coordinate of the lower-left corner of the root rectangle.
     * @param y The y-coordinate of the lower-left corner of the root
     * rectangle.
     * @param width The x-coordinate of the upper-right corner of the root
     * rectangle.
     * @param height The y-coordinate of the upper-right corner of the root
     * rectangle.
     * @param maxChildren Maximum number of children per node (default is 5).
     * @param minChildren Minimum number of children per node (default is 2).
     * @param buffer The amount of padding on each side of the bounding box of each object.
     */
    RTree(float x, float y, float width, float height,
                             unsigned int maxChildren = 5, unsigned int minChildren = 2,
                             float buffer = 20);

    /**
     * Searches for objects within a given circular area.
     *
     * @param center The center of the circle to search.
     * @param radius The radius of the circle to search.
     * @return A vector of shared pointers to RTreeObject instances intersecting
     * the search area.
     */
    std::vector<std::shared_ptr<RTreeObject>> search(const Vec2 center, float radius);

    /**
     * Inserts an object into the R-Tree.
     *
     * @param obj Shared pointer to the RTreeObject to be inserted.
     * @param buffer Optional buffer value to expand the object's bounding box
     * (default is 20).
     */
    void insert(std::shared_ptr<RTreeObject> obj);

    /**
     * Removes an object from this RTree.
     *
     * @param obj Shared pointer to the RTreeObject to be removed.
     */
    void remove(std::shared_ptr<RTreeObject> obj);
    
    /**
     * Bulk inserts a vector of objects.
     *
     * @param objects List of objects to insert.
     */
    void bulkInsert(std::vector<std::shared_ptr<RTreeObject>> objects);

    /**
     * Reconstructs this RTree using all of its existing points.
     */
    void reconstruct();

    /**
     * Updates this RTree depending on the state of its objects.
     *
     * If one of the objects in the RTree is no longer contained in its bounding
     * box, the RTree is reconstructed.
     */
    void update();

    void draw(const std::shared_ptr<SpriteBatch> &batch);
};

#endif
