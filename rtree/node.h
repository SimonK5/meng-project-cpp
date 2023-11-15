#ifndef NODE_H
#define NODE_H

#include <memory>
#include <vector>

#include "geometry.h"

class Node {
 public:
  int level;
  std::shared_ptr<Rect> rect;
  std::vector<std::shared_ptr<Node>> children;
  std::shared_ptr<Node> obj;

  Node(int x1, int y1, int x2, int y2,
       std::vector<std::shared_ptr<Node>> children, int level);

  Node(std::shared_ptr<Rect> r, std::vector<std::shared_ptr<Node>> children,
       int level);

  Node(std::shared_ptr<Rect> r);
};

#endif