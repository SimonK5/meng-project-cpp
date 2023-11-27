#ifndef OBJ_H
#define OBJ_H

#include <memory>
#include <vector>

#include "geometry.h"

class RTreeObject {
 public:
  // The bounding box of this object
  Rect* rect;

  /**
   * @param x1 The x-coordinate of the lower-left corner.
   * @param y1 The y-coordinate of the lower-left corner.
   * @param x2 The x-coordinate of the upper-right corner.
   * @param y2 The y-coordinate of the upper-right corner.
   */
  RTreeObject(int x1, int y1, int x2, int y2);

  ~RTreeObject();
};

#endif