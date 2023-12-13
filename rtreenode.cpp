#include "rtreenode.h"
#include <cugl/cugl.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace cugl;

/**
 * Creates an RTreeNode from coordinates and width/height of the bounding box,
 * a list of children, and the level of the node.
 *
 * @param x1 The x-coord of the lower-left corner of the node's bounding box.
 * @param y1 The y-coord of the lower-left corner of the node's bounding box.
 * @param width The x-coord of the upper-right corner of the node's bounding box.
 * @param height The y-coord of the upper-right corner of the node's bounding box.
 * @param children Vector of pointers to the children nodes of this node.
 * @param level The level of this node in the R-tree.
 */
RTreeNode::RTreeNode(int x1, int y1, int width, int height,
                     std::vector<std::shared_ptr<RTreeNode>> children,
                     int level)
    : level(level), rect(x1, y1, width, height), children(children) {}

/**
 * Creates an RTreeNode from a bounding box, a list of children, and the level of the node.
 *
 * @param r The pointer to the Rect bounding box of the node.
 * @param children The children nodes of this node.
 * @param level The level of this node in the R-tree.
 */
RTreeNode::RTreeNode(Rect r, std::vector<std::shared_ptr<RTreeNode>> children,
                     int level)
    : level(level), rect(r), children(children) {}

/**
 * Creates an RTreeNode from a list of children and a level.
 *
 * @param children The children nodes of this node.
 * @param level The level of this node in the R-tree.
 */
RTreeNode::RTreeNode(std::vector<std::shared_ptr<RTreeNode>> children,
                     int level)
    : level(level), children(children) {
    if (children.size() > 0) {
        rect = Rect(children[0]->rect);
        for (auto it = children.begin(); it != children.end(); ++it) {
            rect += (*it)->rect;
        }
    }
}

/**
 * Creates an RTreeNode from a bounding rectangle and a level.
 *
 * @param children The children nodes of this node.
 * @param level The level of this node in the R-tree.
 */
RTreeNode::RTreeNode(Rect r, int level) : level(level), rect(r) {}

/**
 * Creates an RTreeNode from a bounding rectangle.
 *
 * @param children The children nodes of this node.
 * @param level The level of this node in the R-tree.
 */
RTreeNode::RTreeNode(Rect r) : rect(r) {}

/**
 * Removes a child node from this node.
 *
 * @param c The address of the child node to be removed.
 */
void RTreeNode::deleteChild(const RTreeNode& c) {
    auto it = std::find_if(
        children.begin(), children.end(),
        [&](const std::shared_ptr<RTreeNode>& ptr) { return ptr.get() == &c; });

    if (it != children.end()) {
        children.erase(it);
    }
}

/**
 * Recursively delete all children of this RTreeNode.
 *
 * This does NOT delete any of the RTreeObjects contained in the leaves of the tree.
 */
void RTreeNode::deleteChildren() {
    for (auto& child : children) {
        child->deleteChildren();
    }
    children.clear();
}

/**
 * Adds a child node to this node.
 *
 * @param c The address of the child node to be added.
 */
void RTreeNode::addChild(const std::shared_ptr<RTreeNode>& c) {
    children.push_back(c);
}

/**
 * Return a string representation of this tree.
 *
 * @param height The height of the tree.
 * @return std::string
 */
std::string RTreeNode::print(int height) const {
    std::string res = "";
    std::string indentation = "";
    for (int i = 0; i < height - level; ++i) {
        indentation += " ";
    }
    std::string nodeInfo = indentation + "[(" + std::to_string(rect.getMinX()) +
                            ", " + std::to_string(rect.getMinY()) + "), (" +
                            std::to_string(rect.getMaxX()) + ", " +
                            std::to_string(rect.getMaxY()) + ")]\n";
    res += nodeInfo;
    for (const auto& child : children) {
        res += child->print(height);
    }
    return res;
}

void RTreeNode::draw(const std::shared_ptr<SpriteBatch>& batch) {
    Rect r = Rect((rect.origin.x) / 1024, (rect.origin.y) / 576,
                (rect.size.width) / 1024, (rect.size.height) / 576);
    batch->outline(r);
    for (auto& child : children) {
        child->draw(batch);
    }
}
