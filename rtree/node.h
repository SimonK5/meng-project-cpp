#ifndef NODE_H
#define NODE_H

#include <memory>
#include <vector>

#include "geometry.h"

class Node {
 public:
  int level;
  Rect* rect;
  std::vector<Node*> children;
  Node* obj;

  Node(int x1, int y1, int x2, int y2, std::vector<Node*> children, int level);

  Node(Rect* r, std::vector<Node*> children, int level);

  Node(Rect* r);

 private:
  ~Node();
};

#endif