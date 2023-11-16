#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <memory>

class Rect {
 public:
  int x1, y1, x2, y2;

  Rect(int x1, int y1, int x2, int y2);

  bool contains(const Rect& other) const;
  bool intersects(const Rect& other) const;
  Rect enlargeToContain(const Rect& other) const;
  int area() const;
};

#endif  // GEOMETRY_H
