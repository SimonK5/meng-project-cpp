#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <memory>

class Rect {
 public:
  int x1, y1, x2, y2;

  Rect(int x1, int y1, int x2, int y2);

  bool contains(const std::shared_ptr<Rect> other) const;
  bool intersects(const std::shared_ptr<Rect> other) const;
  std::shared_ptr<Rect> enlargeToContain(
      const std::shared_ptr<Rect> other) const;
  int area() const;
};

#endif  // GEOMETRY_H
