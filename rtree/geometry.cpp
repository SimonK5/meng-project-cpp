#include "geometry.h"

#include <algorithm>
#include <iostream>

Rect::Rect(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}

Rect::Rect(Rect& r) : x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2) {}

bool Rect::contains(const Rect& other) const {
  return (x1 <= other.x1 && y1 <= other.y1 && x2 >= other.x2 && y2 >= other.y2);
}

bool Rect::intersects(const Rect& other) const {
  return !(x2 < other.x1 || x1 > other.x2 || y2 < other.y1 || y1 > other.y2);
}

Rect Rect::enlargeToContain(const Rect& other) const {
  return Rect(std::min(x1, other.x1), std::min(y1, other.y1),
              std::max(x2, other.x2), std::max(y2, other.y2));
}

int Rect::area() const { return std::abs(x2 - x1) * std::abs(y2 - y1); }
