#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <memory>

class Rect {
 public:
  // Coordinates of the lower-left corner
  int x1, y1;
  // Coordinates of the upper-right corner
  int x2, y2;

  /**
   * @param x1 The x-coordinate of the lower-left corner.
   * @param y1 The y-coordinate of the lower-left corner.
   * @param x2 The x-coordinate of the upper-right corner.
   * @param y2 The y-coordinate of the upper-right corner.
   */
  Rect(int x1, int y1, int x2, int y2);

  /**
   * @param r The reference to the Rect object to be copied.
   */
  Rect(Rect& r);

  /**
   * Determine whether another Rect is contained in this Rect.
   *
   * @param other Rect to compare to this Rect.
   * @return true if other is contained in this Rect.
   */
  bool contains(const Rect& other) const;

  /**
   * Checks whether another Rect intersects with this Rect.
   *
   * @param other The Rect to check for intersection with this Rect.
   * @return true if 'other' intersects with this Rect, otherwise false.
   */
  bool intersects(const Rect& other) const;

  /**
   * Enlarges this Rect to contain another Rect and returns the enlarged Rect.
   *
   * @param other The Rect to be contained within the enlarged Rect.
   * @return The enlarged Rect that contains both this Rect and 'other'.
   */
  Rect enlargeToContain(const Rect& other) const;

  /**
   * Calculates the area of this Rect.
   *
   * @return The area (integer value) enclosed by this Rect.
   */
  int area() const;
};

#endif  // GEOMETRY_H
