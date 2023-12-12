#ifndef OBJ_H
#define OBJ_H

#include <memory>
#include <random>
#include <string>
#include <vector>
#include <cugl/cugl.h>

using namespace cugl;

class RTreeObject {
public:
    // The bounding box of this object
    Rect rect;
    float velX;
    float velY;

    /**
    * @param x1 The x-coordinate of the lower-left corner.
    * @param y1 The y-coordinate of the lower-left corner.
    * @param width The x-coordinate of the upper-right corner.
    * @param height The y-coordinate of the upper-right corner.
    */
    RTreeObject(float x1, float y1, float width, float height);

    void update(float mapWidth, float mapHeight);

    std::string print();
    
    void draw(const std::shared_ptr<SpriteBatch>& batch);
};

#endif
