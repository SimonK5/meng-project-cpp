#ifndef NODE_H
#define NODE_H

#include <memory>
#include <string>
#include <vector>
#include "rtreeobject.h"
#include <cugl/cugl.h>

using namespace cugl;

/**
 * Class representing a node of an R-tree.
 */
class RTreeNode {
public:
    /** The level of this node in the R-tree. Leaf nodes have a level of -1. */
    int level;
    /** The bounding box of this node. */
    Rect rect;
    /** The children of this node, if it is an inner node. */
    std::vector<std::shared_ptr<RTreeNode>> children;
    /**  The object contained by this node, if it is a leaf node. */
    std::shared_ptr<RTreeObject> obj;

    /**
     * Removes a child node from this node.
     *
     * @param c The address of the child node to be removed.
     */
    void deleteChild(const RTreeNode& c);
    
    /**
     * Recursively deletes all children of this RTreeNode.
     *
     * This does NOT delete any of the RTreeObjects contained in the leaves of the tree.
     */
    void deleteChildren();

    /**
     * Adds a child node to this node.
     *
     * @param c The address of the child node to be added.
     */
    void addChild(const std::shared_ptr<RTreeNode>& c);

    /**
     * Returns a string representation of this tree.
     *
     * @param height The height of the tree.
     * @return std::string
     */
    std::string print(int height) const;

    /**
     * Creates a new RTreeNode object from coordinates and width/height of the bounding box,
     * a list of children, and the level of the node.
     *
     * @param x1 The x-coord of the lower-left corner of the node's bounding box.
     * @param y1 The y-coord of the lower-left corner of the node's bounding box.
     * @param width The x-coord of the upper-right corner of the node's bounding box.
     * @param height The y-coord of the upper-right corner of the node's bounding box.
     * @param children Vector of pointers to the children nodes of this node.
     * @param level The level of this node in the R-tree.
     */
    RTreeNode(int x1, int y1, int width, int height,
              std::vector<std::shared_ptr<RTreeNode>> children, int level);

    /**
     * Creates an RTreeNode from a bounding box, a list of children, and the level of the node.
     *
     * @param r The pointer to the Rect bounding box of the node.
     * @param children The children nodes of this node.
     * @param level The level of this node in the R-tree.
     */
    RTreeNode(Rect r, std::vector<std::shared_ptr<RTreeNode>> children, int level);
    
    /**
     * Creates an RTreeNode from a list of children and a level.
     *
     * @param children The children nodes of this node.
     * @param level The level of this node in the R-tree.
     */
    RTreeNode(std::vector<std::shared_ptr<RTreeNode>> children, int level);
    
    /**
     * Creates an RTreeNode from a bounding rectangle and a level.
     *
     * @param children The children nodes of this node.
     * @param level The level of this node in the R-tree.
     */
    RTreeNode(Rect r, int level);

    /**
     * Creates an RTreeNode from a bounding rectangle.
     *
     * @param children The children nodes of this node.
     * @param level The level of this node in the R-tree.
     */
    RTreeNode(Rect r);

    void draw(const std::shared_ptr<SpriteBatch>& batch);
};

#endif
