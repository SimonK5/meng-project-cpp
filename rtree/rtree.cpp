#include "rtree.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "rtreenode.h"
#include "rtreeobject.h"

#include <cugl/cugl.h>

using namespace cugl;

void RTree::clear() {
    root->deleteChildren();
  root = std::make_shared<RTreeNode>(rect.getMinX(), rect.getMinY(), rect.getMaxX(),
                                     rect.getMaxY(), std::vector<std::shared_ptr<RTreeNode>>{}, 0);
}

void RTree::find_intersections(RTreeNode &n, const Vec2 center, float radius,
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
        find_intersections(*child, center, radius, res);
      }
    }
  }
}

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTree::pick_seeds(RTreeNode &n) {
  std::shared_ptr<RTreeNode> max_low_side_entry_x = nullptr;
  std::shared_ptr<RTreeNode> min_high_side_entry_x = nullptr;
  std::shared_ptr<RTreeNode> max_low_side_entry_y = nullptr;
  std::shared_ptr<RTreeNode> min_high_side_entry_y = nullptr;

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    std::shared_ptr<RTreeNode> child = *it;
    if ((max_low_side_entry_x == nullptr ||
         child->rect.getMinX() > max_low_side_entry_x->rect.getMinX()) &&
        child != min_high_side_entry_x) {
      max_low_side_entry_x = child;
    }
  }

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    std::shared_ptr<RTreeNode> child = *it;
    if ((min_high_side_entry_x == nullptr ||
         child->rect.getMaxX() < min_high_side_entry_x->rect.getMaxX()) &&
        child != max_low_side_entry_x) {
      min_high_side_entry_x = child;
    }
  }

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    std::shared_ptr<RTreeNode> child = *it;
    if ((max_low_side_entry_y == nullptr ||
         child->rect.getMinY() > max_low_side_entry_y->rect.getMinY()) &&
        child != min_high_side_entry_y) {
      max_low_side_entry_y = child;
    }
  }

  for (auto it = n.children.begin(); it != n.children.end(); ++it) {
    std::shared_ptr<RTreeNode> child = *it;
    if ((min_high_side_entry_y == nullptr ||
         child->rect.getMaxY() < min_high_side_entry_y->rect.getMaxY()) &&
        child != max_low_side_entry_y) {
      min_high_side_entry_y = child;
    }
  }

  double separation_x = (double)(min_high_side_entry_x->rect.getMaxX() -
                                 max_low_side_entry_x->rect.getMinX()) /
                        (n.rect.getMaxX() - n.rect.getMinX());
  double separation_y = (double)(min_high_side_entry_y->rect.getMaxY() -
                                 max_low_side_entry_y->rect.getMinY()) /
                        (n.rect.getMaxY() - n.rect.getMinY());

  std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> seeds;
  if (separation_y > separation_x) {
    seeds.first = max_low_side_entry_y;
    seeds.second = min_high_side_entry_y;
  } else {
    seeds.first = max_low_side_entry_x;
    seeds.second = min_high_side_entry_x;
  }

  return seeds;
}

std::shared_ptr<RTreeNode> RTree::pick_next(const std::vector<std::shared_ptr<RTreeNode>> &children,
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

std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> RTree::linear_split(RTreeNode &n) {
  std::pair<std::shared_ptr<RTreeNode> , std::shared_ptr<RTreeNode> > seeds = pick_seeds(n);

  std::shared_ptr<RTreeNode> c1 = seeds.first;
  std::shared_ptr<RTreeNode> c2 = seeds.second;

  std::unordered_set<std::shared_ptr<RTreeNode> > added;
    std::shared_ptr<RTreeNode> node1 = std::make_shared<RTreeNode>(c1->rect, std::vector<std::shared_ptr<RTreeNode>>{}, n.level);
    std::shared_ptr<RTreeNode> node2 = std::make_shared<RTreeNode>(c2->rect, std::vector<std::shared_ptr<RTreeNode>>{}, n.level);

  node1->children.push_back(c1);
  node2->children.push_back(c2);
  added.insert(c2);
  added.insert(c1);
  while (added.size() < n.children.size()) {
    std::shared_ptr<RTreeNode> nextNode =
        pick_next(n.children, added, node1->rect, node2->rect);

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

Rect RTree::get_enlarged_bb(RTreeNode &n, std::shared_ptr<RTreeNode> &best_child,
                             const Rect &container_rect) {
  float min_area_increase = INT32_MAX;
  Rect best_bb = Rect(-1, -1, -1, -1);

  for (auto &child : n.children) {
    Rect r = child->rect;
    Rect enlargedBB = r.getMerge(container_rect);
    float areaEnlarged = enlargedBB.size.width * enlargedBB.size.height;
    float areaChild = r.size.width * r.size.height;
    float area_increase = areaEnlarged - areaChild;

    if (area_increase < min_area_increase) {
      min_area_increase = area_increase;
      best_bb = Rect(enlargedBB);
      best_child = child;
    }
  }

  return best_bb;
}

void RTree::insert_helper(RTreeNode &n, std::shared_ptr<RTreeObject> &obj) {
  Rect container_rect = Rect(obj->rect.getMinX() - bufferSize, obj->rect.getMinY() - bufferSize,
               obj->rect.size.width + bufferSize * 2, obj->rect.size.height + bufferSize * 2);

  if (n.level > 0) {
    std::shared_ptr<RTreeNode> best_child = nullptr;
    bool fits_in_child = false;
    for (auto it = n.children.begin(); it != n.children.end(); ++it) {
      Rect r = (*it)->rect;
      if (r.contains(container_rect)) {
        insert_helper(*(*it), obj);
        fits_in_child = true;
        best_child = (*it);
        break;
      }
    }

    // If no child node can fit this object, expand one of them to fit it
    if (!fits_in_child) {
      best_child->rect = get_enlarged_bb(n, best_child, container_rect);
      insert_helper(*best_child, obj);
    }
    if ((*best_child).children.size() > maxPerLevel) {
      std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> nodes = linear_split(*best_child);
      n.deleteChild(*best_child);
      n.addChild(nodes.first);
      n.addChild(nodes.second);
    }
  } else {
    std::vector<std::shared_ptr<RTreeNode>> emptyVector;
    std::shared_ptr<RTreeNode> container_node = std::make_shared<RTreeNode>(container_rect, emptyVector, -1);
    container_node->obj = obj;
    n.children.push_back(container_node);
      particleToBBox.insert(std::make_pair(obj, container_rect));
  }
}

std::vector<std::shared_ptr<RTreeObject>> RTree::removeHelper(std::shared_ptr<RTreeNode> &n, std::shared_ptr<RTreeObject> &obj){
    std::vector<std::shared_ptr<RTreeObject>> entriesToReinsert;
    
    if(n->level == 0){
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
    
    else{
        bool mustResize = false;
        auto it = n->children.begin();
        while (it != n->children.end()) {
            auto entries = removeHelper((*it), obj);
            if (entries.size() > 0) {
                entriesToReinsert = entries;

                // Erase the current child from the vector and obtain the iterator to the next element
                it = n->children.erase(it);
                mustResize = true;
            } else {
                ++it;
            }
        }
        
        if(mustResize){
            Rect newBBox = n->children[0]->rect;
            for (auto it = n->children.begin(); it != n->children.end(); ++it) {
                newBBox += (*it)->rect;
            }
            n->rect = newBBox;
        }
    }
    
    return entriesToReinsert;
}

RTree::RTree(float x1, float y1, float width, float height,
             unsigned int max_per_level, unsigned int min_per_level, float buffer)
    : rect(Rect(x1, y1, width, height)),
      maxPerLevel(max_per_level),
        minPerLevel(min_per_level),
        bufferSize(buffer),
root(std::make_shared<RTreeNode>(x1, y1, width, height, std::vector<std::shared_ptr<RTreeNode>>{}, 0)),
        particleToBBox() {};

std::vector<std::shared_ptr<RTreeObject>> RTree::search(const Vec2 center, float radius) {
  std::vector<std::shared_ptr<RTreeObject>> res;
  find_intersections(*root, center, radius, res);
  return res;
}

void RTree::insert(std::shared_ptr<RTreeObject> obj) {
  insert_helper(*root, obj);
  if (root->children.size() > maxPerLevel) {
      std::shared_ptr<RTreeNode> newRoot = std::make_shared<RTreeNode>(root->rect, std::vector<std::shared_ptr<RTreeNode>>{}, root->level + 1);
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> nodes = linear_split(*root);
    newRoot->addChild(nodes.first);
    newRoot->addChild(nodes.second);
    root = newRoot;
  }
}

void RTree::remove(std::shared_ptr<RTreeObject> obj) {
    std::vector<std::shared_ptr<RTreeObject>> toReinsert = removeHelper(root, obj);
    for(auto it = toReinsert.begin(); it != toReinsert.end(); ++it){
        insert(*it);
    }
    
    if(root->children.size() == 1 && root->level > 0){
        Rect prevBBox = root->rect;
        root = root->children[0];
        root->rect = prevBBox;
    }
    
    particleToBBox.erase(obj);
}

void RTree::reconstruct(){
    root->deleteChildren();
    root = std::make_shared<RTreeNode>(rect.getMinX(), rect.getMinY(), rect.getMaxX(),
                         rect.getMaxY(), std::vector<std::shared_ptr<RTreeNode>>{}, 0);
    
    std::unordered_map<std::shared_ptr<RTreeObject>, Rect> particleToBBoxOld = particleToBBox;
    particleToBBox = {};
    for (auto it = particleToBBoxOld.begin(); it != particleToBBoxOld.end(); ++it) {
        std::shared_ptr<RTreeObject> obj = it->first;
        insert(obj);
    }
}

void RTree::update(){
    auto test = particleToBBox.begin();
    
    for (auto it = particleToBBox.begin(); it != particleToBBox.end(); ++it) {
        Rect objectRect = it->first->rect;
        Rect bboxRect = it->second;
        
        if(!objectRect.inside(bboxRect)){
            reconstruct();
            break;
        }
    }
}

void RTree::draw(const std::shared_ptr<SpriteBatch>& batch){
    root->draw(batch);
}
