#include "geometry.h"

#include <algorithm>
#include <iostream>

Rect::Rect(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}

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

// int main() {
//   Rect * rect1 = std::make_shared<Rect>(1, 1, 5, 5);
//   Rect * rect2 = std::make_shared<Rect>(3, 3, 6, 6);

//   std::cout << "Rect1 contains Rect2: " << std::boolalpha
//             << rect1->contains(rect2) << std::endl;
//   std::cout << "Rect1 intersects Rect2: " << std::boolalpha
//             << rect1->intersects(rect2) << std::endl;

//   Rect enlargedRect = rect1->enlargeToContain(rect2);
//   std::cout << "Enlarged Rectangle: (" << enlargedRect.x1 << ", "
//             << enlargedRect.y1 << ") - (" << enlargedRect.x2 << ", "
//             << enlargedRect.y2 << ")" << std::endl;

//   std::cout << "Area of Rect1: " << rect1->area() << std::endl;

//   return 0;
// }
