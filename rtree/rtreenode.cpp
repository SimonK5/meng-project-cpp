#include "rtreenode.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cugl/cugl.h>

using namespace cugl;

RTreeNode::RTreeNode(int x1, int y1, int width, int height,
                     std::vector<std::shared_ptr<RTreeNode>> children, int level)
    : level(level), rect(x1, y1, width, height), children(children) {}

RTreeNode::RTreeNode(Rect r, std::vector<std::shared_ptr<RTreeNode>> children, int level)
    : level(level), rect(r), children(children) {}

RTreeNode::RTreeNode(Rect r) : rect(r) {}

void RTreeNode::deleteChildren() {
  for (auto& child : children) {
    child->deleteChildren();
  }
  children.clear();
}

void RTreeNode::deleteChild(const RTreeNode& c) {
  auto it = std::find_if(children.begin(), children.end(),
                         [&](const std::shared_ptr<RTreeNode>& ptr) { return ptr.get() == &c; });

  if (it != children.end()) {
    children.erase(it);
  }
}

void RTreeNode::addChild(const std::shared_ptr<RTreeNode>& c) {
  children.push_back(c);
}

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
  for (const auto &child : children) {
    res += child->print(height);
  }
  return res;
}

void RTreeNode::draw(const std::shared_ptr<SpriteBatch>& batch){
    Rect r = Rect((rect.origin.x)/1024, (rect.origin.y)/576, (rect.size.width)/1024, (rect.size.height)/576);
    batch->outline(r);
    for(auto& child : children) {
        child->draw(batch);
    }
}
