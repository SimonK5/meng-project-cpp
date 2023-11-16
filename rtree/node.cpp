#include "node.h"

#include <memory>
#include <vector>

#include "geometry.h"

Node::Node(int x1, int y1, int x2, int y2, std::vector<Node *> children,
           int level)
    : level(level), rect(new Rect(x1, y1, x2, y2)), children(children) {}

Node::Node(Rect *r, std::vector<Node *> children, int level)
    : level(level), rect(r), children(children) {}

Node::Node(Rect *r) : rect(r) {}

Node::~Node() {
  delete rect;

  for (Node *child : children) {
    delete child;
  }
}

// int main() { Node node1(std::make_shared<Rect>(10, 10, 30, 30), {}, 0); }