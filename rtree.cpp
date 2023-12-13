#include "rtree.h"

#include <cugl/cugl.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "rtreenode.h"
#include "rtreeobject.h"

using namespace cugl;
/**
 * Fills a vector with objects in a subtree that intersect with a given
 * circular area.
 *
 * @param n The root of the subtree.
 * @param center The center of the circle.
 * @param radius The radius of the circle.
 * @param res Vector containing objects that intersect the area.
 */
void RTree::findIntersections(RTreeNode &n, const Vec2 center, float radius,
                            std::vector<std::shared_ptr<RTreeObject>> &res) {
    if (n.level == 0) {
        for (auto &child : n.children) {
            if (child->obj->rect.doesIntersect(center, radius)) {
                res.push_back(child->obj);
            }
        }
    } else {
        for (auto &child : n.children) {
            if (child->rect.doesIntersect(center, radius)) {
                findIntersections(*child, center, radius, res);
            }
        }
    }
}

/**
 * Given a node to split, selects two children of the original node to become
 * the first children of the two new nodes.
 *
 * @param n The node to split
 * @return Pair of RTreeNode shared pointers containing the first two elements of the two new nodes
 */
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTree::pickSeeds(RTreeNode &n) {
    std::shared_ptr<RTreeNode> maxLowSideEntryX = nullptr;
    std::shared_ptr<RTreeNode> minHighSideEntryX = nullptr;
    std::shared_ptr<RTreeNode> maxLowSideEntryY = nullptr;
    std::shared_ptr<RTreeNode> minHighSideEntryY = nullptr;

    for (auto it = n.children.begin(); it != n.children.end(); ++it) {
        std::shared_ptr<RTreeNode> child = *it;
        if ((maxLowSideEntryX == nullptr ||
                 child->rect.getMinX() > maxLowSideEntryX->rect.getMinX()) &&
                child != minHighSideEntryX) {
            maxLowSideEntryX = child;
        }
    }

    for (auto it = n.children.begin(); it != n.children.end(); ++it) {
        std::shared_ptr<RTreeNode> child = *it;
        if ((minHighSideEntryX == nullptr ||
                 child->rect.getMaxX() < minHighSideEntryX->rect.getMaxX()) &&
                child != maxLowSideEntryX) {
            minHighSideEntryX = child;
        }
    }

    for (auto it = n.children.begin(); it != n.children.end(); ++it) {
        std::shared_ptr<RTreeNode> child = *it;
        if ((maxLowSideEntryY == nullptr ||
                 child->rect.getMinY() > maxLowSideEntryY->rect.getMinY()) &&
                child != minHighSideEntryY) {
            maxLowSideEntryY = child;
        }
    }

    for (auto it = n.children.begin(); it != n.children.end(); ++it) {
        std::shared_ptr<RTreeNode> child = *it;
        if ((minHighSideEntryY == nullptr ||
                 child->rect.getMaxY() < minHighSideEntryY->rect.getMaxY()) &&
                child != maxLowSideEntryY) {
            minHighSideEntryY = child;
        }
    }

    double separationX = (double)(minHighSideEntryX->rect.getMaxX() -
                                                                 maxLowSideEntryX->rect.getMinX()) /
                                                (n.rect.getMaxX() - n.rect.getMinX());
    double separationY = (double)(minHighSideEntryY->rect.getMaxY() -
                                                                 maxLowSideEntryY->rect.getMinY()) /
                                                (n.rect.getMaxY() - n.rect.getMinY());

    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> seeds;
    if (separationY > separationX) {
        seeds.first = maxLowSideEntryY;
        seeds.second = minHighSideEntryY;
    } else {
        seeds.first = maxLowSideEntryX;
        seeds.second = minHighSideEntryX;
    }

    return seeds;
}

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
std::shared_ptr<RTreeNode> RTree::pickNext(
        const std::vector<std::shared_ptr<RTreeNode>> &children,
        const std::unordered_set<std::shared_ptr<RTreeNode>> &added,
        const Rect &bbox_1, const Rect &bbox_2) {
    int max_diff = 0;
    std::shared_ptr<RTreeNode> max_child = nullptr;
    for (auto it = children.begin(); it != children.end(); ++it) {
        std::shared_ptr<RTreeNode> child = *it;
        if (added.find(child) != added.end()) {
            continue;
        }

        Rect enlarged1 = bbox_1.getMerge(child->rect);
        Rect enlarged2 = bbox_2.getMerge(child->rect);
        float area1 = enlarged1.size.width * enlarged1.size.height;
        float area2 = enlarged2.size.width * enlarged2.size.height;
        float diff = std::abs(area1 - area2);

        if (max_child == nullptr || diff > max_diff) {
            max_child = child;
            max_diff = diff;
        }
    }

    return max_child;
}

/**
 * Splits an overflowing node into two nodes.
 *
 * @param n The node to be split.
 * @return The two resulting nodes from the split.
 */
std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTree::linearSplit(RTreeNode &n) {
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> seeds =
            pickSeeds(n);

    std::shared_ptr<RTreeNode> c1 = seeds.first;
    std::shared_ptr<RTreeNode> c2 = seeds.second;

    std::unordered_set<std::shared_ptr<RTreeNode>> added;
    std::shared_ptr<RTreeNode> node1 = std::make_shared<RTreeNode>(
            c1->rect, std::vector<std::shared_ptr<RTreeNode>>{}, n.level);
    std::shared_ptr<RTreeNode> node2 = std::make_shared<RTreeNode>(
            c2->rect, std::vector<std::shared_ptr<RTreeNode>>{}, n.level);

    node1->children.push_back(c1);
    node2->children.push_back(c2);
    added.insert(c2);
    added.insert(c1);
    while (added.size() < n.children.size()) {
        std::shared_ptr<RTreeNode> nextNode =
                pickNext(n.children, added, node1->rect, node2->rect);

        added.insert(nextNode);
        Rect enlarged1 = node1->rect.getMerge(nextNode->rect);
        Rect enlarged2 = node2->rect.getMerge(nextNode->rect);
        float area1 = enlarged1.size.width * enlarged1.size.height;
        float area2 = enlarged2.size.width * enlarged2.size.height;
        if (area1 < area2) {
            node1->children.push_back(nextNode);
            node1->rect = Rect(enlarged1);
        } else {
            node2->children.push_back(nextNode);
            node2->rect = Rect(enlarged2);
        }
    }
    return std::make_pair(node1, node2);
}

/**
 * Given a rectangle, determine the child bounding box such that the union of the new rectangle and
 * child bounding box is minimal.
 *
 * @param n The parent of the candidate child nodes to be checked.
 * @param containerRect The bounding box of the object to be inserted
 * @return The node that can expand to fit containerRect with minimal area increase.
 */
std::shared_ptr<RTreeNode> RTree::findBestBB(RTreeNode &n, const Rect &containerRect) {
    float minAreaIncrease = INT32_MAX;
    Rect best_bb = Rect(-1, -1, -1, -1);
    std::shared_ptr<RTreeNode> bestChild = nullptr;

    for (auto &child : n.children) {
        Rect r = child->rect;
        Rect enlargedBB = r.getMerge(containerRect);
        float areaEnlarged = enlargedBB.size.width * enlargedBB.size.height;
        float areaChild = r.size.width * r.size.height;
        float areaIncrease = areaEnlarged - areaChild;

        if (areaIncrease < minAreaIncrease) {
            minAreaIncrease = areaIncrease;
            bestChild = child;
        }
    }

    return bestChild;
}

/**
 * Inserts an object into a node.
 *
 * @param n The node into which the object will be inserted.
 * @param obj The object to insert.
 */
void RTree::insertHelper(RTreeNode &n, std::shared_ptr<RTreeObject> &obj) {
    Rect containerRect =
            Rect(obj->rect.getMinX() - bufferSize, obj->rect.getMinY() - bufferSize,
                     obj->rect.size.width + bufferSize * 2,
                     obj->rect.size.height + bufferSize * 2);

    if (n.level > 0) {
        std::shared_ptr<RTreeNode> bestChild = nullptr;
        bool fitsInChild = false;
        for (auto it = n.children.begin(); it != n.children.end(); ++it) {
            Rect r = (*it)->rect;
            if (r.contains(containerRect)) {
                insertHelper(*(*it), obj);
                fitsInChild = true;
                bestChild = (*it);
                break;
            }
        }

        // If no child node can fit this object, expand one of them to fit it
        if (!fitsInChild) {
            bestChild = findBestBB(n, containerRect);
            bestChild->rect += containerRect;
            insertHelper(*bestChild, obj);
        }
        if ((*bestChild).children.size() > maxPerLevel) {
            std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> nodes =
                    linearSplit(*bestChild);
            n.deleteChild(*bestChild);
            n.addChild(nodes.first);
            n.addChild(nodes.second);
        }
    } else {
        std::vector<std::shared_ptr<RTreeNode>> emptyVector;
        std::shared_ptr<RTreeNode> containerNode =
                std::make_shared<RTreeNode>(containerRect, emptyVector, -1);
        containerNode->obj = obj;
        n.children.push_back(containerNode);
        objectToBBox.insert(std::make_pair(obj, containerRect));
    }
}

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
std::vector<std::shared_ptr<RTreeObject>> RTree::removeHelper(
        std::shared_ptr<RTreeNode> &n, std::shared_ptr<RTreeObject> &obj) {
    std::vector<std::shared_ptr<RTreeObject>> entriesToReinsert;

    if (n->level == 0) {
        for (auto it = n->children.begin(); it != n->children.end(); ++it) {
            if ((*it)->obj.get() == obj.get()) {
                n->children.erase(it);
                break;
            }
        }

        if (n->children.size() < minPerLevel) {
            for (auto it = n->children.begin(); it != n->children.end(); ++it) {
                entriesToReinsert.push_back((*it)->obj);
            }
            n->children.clear();
        }
    }

    else {
        bool mustResize = false;
        auto it = n->children.begin();
        while (it != n->children.end()) {
            auto entries = removeHelper((*it), obj);
            if (entries.size() > 0) {
                entriesToReinsert = entries;

                // Erase the current child from the vector and obtain the iterator to
                // the next element
                it = n->children.erase(it);
                mustResize = true;
            } else {
                ++it;
            }
        }

        if (mustResize) {
            Rect newBBox = n->children[0]->rect;
            for (auto it = n->children.begin(); it != n->children.end(); ++it) {
                newBBox += (*it)->rect;
            }
            n->rect = newBBox;
        }
    }

    return entriesToReinsert;
}

/**
 * Partition a list of child nodes into a certain amount of new parent nodes.
 *
 * @param nodes Vector of nodes to be partitioned
 * @return Vector of the new children nodes
 */
std::vector<std::shared_ptr<RTreeNode>> RTree::strSplit(
        std::vector<std::shared_ptr<RTreeNode>> &nodes, int level) {
    std::vector<std::shared_ptr<RTreeNode>> parents;
    std::sort(nodes.begin(), nodes.end(),
                        [](const std::shared_ptr<RTreeNode> &a,
                             const std::shared_ptr<RTreeNode> &b) {
                            return a->rect.getMidX() < b->rect.getMidX();
                        });

    int numLeafNodes = std::ceil(nodes.size() / (float)maxPerLevel);
    int numSlices = std::ceil(std::sqrt(numLeafNodes));
    int nodesPerSlice = numSlices * maxPerLevel;

    for (int i = 0; i < numSlices; ++i) {
        std::vector<std::shared_ptr<RTreeNode>> sliceNodes;

        for (int j = i * nodesPerSlice;
                 j < std::min((i + 1) * nodesPerSlice, (int)nodes.size()); ++j) {
            sliceNodes.push_back(nodes[j]);
        }

        std::sort(sliceNodes.begin(), sliceNodes.end(),
                            [](const std::shared_ptr<RTreeNode> &a,
                                 const std::shared_ptr<RTreeNode> &b) {
                                return a->rect.getMidY() < b->rect.getMidY();
                            });

        auto it = sliceNodes.begin();
        while (it != sliceNodes.end()) {
            auto end = std::next(it, maxPerLevel);
            if (std::distance(it, sliceNodes.end()) < maxPerLevel) {
                end = sliceNodes.end();
            }
            std::vector<std::shared_ptr<RTreeNode>> children(it, end);
            std::shared_ptr<RTreeNode> parent =
                    std::make_shared<RTreeNode>(children, level);
            parents.push_back(parent);
            it = end;
        }
    }

    return parents;
}

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
std::shared_ptr<RTreeNode> RTree::sortTileRecursive(
        std::vector<std::shared_ptr<RTreeNode>> &nodes) {
    std::vector<std::shared_ptr<RTreeNode>> parents = strSplit(nodes, 0);

    int level = 1;
    while (parents.size() > 1) {
        parents = strSplit(parents, level);
        level += 1;
    }

    std::shared_ptr<RTreeNode> newRoot = parents[0];
    newRoot->rect = root->rect;

    return parents[0];
}

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
RTree::RTree(float x, float y, float width, float height,
                         unsigned int maxChildren, unsigned int minChildren,
                         float buffer)
        : rect(Rect(x, y, width, height)),
            maxPerLevel(maxChildren),
            minPerLevel(minChildren),
            bufferSize(buffer),
            root(std::make_shared<RTreeNode>(
                    x, y, width, height, std::vector<std::shared_ptr<RTreeNode>>{}, 0)),
            objectToBBox(){};

/**
 * Resets to an empty RTree.
 */
void RTree::clear() {
    root->deleteChildren();
    root = std::make_shared<RTreeNode>(
            rect.getMinX(), rect.getMinY(), rect.getMaxX(), rect.getMaxY(),
            std::vector<std::shared_ptr<RTreeNode>>{}, 0);
}

/**
 * Searches for objects within a given circular area.
 *
 * @param center The center of the circle to search.
 * @param radius The radius of the circle to search.
 * @return A vector of shared pointers to RTreeObject instances intersecting
 * the search area.
 */
std::vector<std::shared_ptr<RTreeObject>> RTree::search(const Vec2 center, float radius) {
    std::vector<std::shared_ptr<RTreeObject>> res;
    findIntersections(*root, center, radius, res);
    return res;
}

/**
 * Inserts an object into the R-Tree.
 *
 * @param obj Shared pointer to the RTreeObject to be inserted.
 * @param buffer Optional buffer value to expand the object's bounding box
 * (default is 20).
 */
void RTree::insert(std::shared_ptr<RTreeObject> obj) {
    insertHelper(*root, obj);
    if (root->children.size() > maxPerLevel) {
        std::shared_ptr<RTreeNode> newRoot = std::make_shared<RTreeNode>(
                root->rect, std::vector<std::shared_ptr<RTreeNode>>{}, root->level + 1);
        std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> nodes =
                linearSplit(*root);
        newRoot->addChild(nodes.first);
        newRoot->addChild(nodes.second);
        root = newRoot;
    }
}

/**
 * Removes an object from this RTree.
 *
 * @param obj Shared pointer to the RTreeObject to be removed.
 */
void RTree::remove(std::shared_ptr<RTreeObject> obj) {
    std::vector<std::shared_ptr<RTreeObject>> toReinsert =
            removeHelper(root, obj);
    for (auto it = toReinsert.begin(); it != toReinsert.end(); ++it) {
        insert(*it);
    }

    if (root->children.size() == 1 && root->level > 0) {
        Rect prevBBox = root->rect;
        root = root->children[0];
        root->rect = prevBBox;
    }

    objectToBBox.erase(obj);
}

/**
 * Bulk inserts a vector of objects.
 *
 * @param objects List of objects to insert.
 */
void RTree::bulkInsert(std::vector<std::shared_ptr<RTreeObject>> objects) {
    std::vector<std::shared_ptr<RTreeNode>> nodes;
    std::unordered_map<std::shared_ptr<RTreeObject>, Rect> newMap;
    objectToBBox.swap(newMap);
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        auto obj = *it;
        Rect containerRect =
                Rect(obj->rect.getMinX() - bufferSize, obj->rect.getMinY() - bufferSize,
                         obj->rect.size.width + bufferSize * 2,
                         obj->rect.size.height + bufferSize * 2);
        std::vector<std::shared_ptr<RTreeNode>> emptyVector;
        std::shared_ptr<RTreeNode> containerNode =
                std::make_shared<RTreeNode>(containerRect, emptyVector, -1);
        containerNode->obj = obj;
        nodes.push_back(containerNode);
        objectToBBox.insert(std::make_pair(obj, containerRect));
    }

    std::shared_ptr<RTreeNode> newRoot = sortTileRecursive(nodes);

    root = newRoot;
}

/**
 * Reconstructs this RTree using all of its existing points.
 */
void RTree::reconstruct() {
    root->deleteChildren();
    std::vector<std::shared_ptr<RTreeObject>> objects;
    for (auto it = objectToBBox.begin(); it != objectToBBox.end(); ++it) {
        objects.push_back(it->first);
    }
    bulkInsert(objects);
}

/**
 * Updates this RTree depending on the state of its objects.
 *
 * If one of the objects in the RTree is no longer contained in its bounding
 * box, the RTree is reconstructed.
 */
void RTree::update() {
    for (auto it = objectToBBox.begin(); it != objectToBBox.end(); ++it) {
        Rect objectRect = it->first->rect;
        Rect bboxRect = it->second;

        if (!objectRect.inside(bboxRect)) {
            reconstruct();
            break;
        }
    }
}

void RTree::draw(const std::shared_ptr<SpriteBatch> &batch) {
    root->draw(batch);
}
