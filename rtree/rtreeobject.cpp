#include "rtreeobject.h"

RTreeObject::RTreeObject(int x1, int y1, int x2, int y2) {
  rect = new Rect(x1, y1, x2, y2);
  // obj_ref = ref;
}

RTreeObject::~RTreeObject() {
  delete rect;
  // delete obj_ref;
}