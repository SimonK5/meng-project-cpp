#ifndef OBJ_H
#define OBJ_H

#include <memory>
#include <vector>

#include "geometry.h"

class RTreeObject {
 public:
  int level;
  Rect* rect;
  RTreeObject(int x1, int y1, int x2, int y2);

  ~RTreeObject();
};

#endif