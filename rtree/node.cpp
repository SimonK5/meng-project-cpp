#include "node.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "geometry.h"

Node::Node(int x1, int y1, int x2, int y2, std::vector<Node *> children,
           int level)
    : level(level), rect(new Rect(x1, y1, x2, y2)), children(children) {}

Node::Node(Rect *r, std::vector<Node *> children, int level)
    : level(level), rect(new Rect(*r)), children(children) {}

Node::Node(Rect *r) : rect(new Rect(*r)) {}

Node::~Node() {
  delete rect;

  for (Node *child : children) {
    delete child;
  }
}

void Node::deleteChild(Node &c) {
  auto it = std::find(children.begin(), children.end(), &c);

  if (it != children.end()) {
    children.erase(it);
    // delete &c;
  }
}

void Node::addChild(Node &c) { children.push_back(&c); }

std::string Node::print(int height) const {
  std::string res = "";
  std::string indentation = "";
  for (int i = 0; i < height - level; ++i) {
    indentation += " ";
  }
  std::string nodeInfo = indentation + "[(" + std::to_string(rect->x1) + ", " +
                         std::to_string(rect->y1) + "), (" +
                         std::to_string(rect->x2) + ", " +
                         std::to_string(rect->y2) + ")]\n";
  res += nodeInfo;
  for (const auto &child : children) {
    res += child->print(height);
  }

  return res;
}
