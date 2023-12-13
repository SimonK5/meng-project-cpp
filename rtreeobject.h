#ifndef OBJ_H
#define OBJ_H

#include <memory>
#include <random>
#include <string>
#include <vector>
#include <cugl/cugl.h>

using namespace cugl;

/**
 * Container class for objects stored in the RTree that specifies its bounding box.
 */
class RTreeObject {
public:
    /** The bounding box of this object. */
    Rect rect;
    
    float velX; // REMOVE BEFORE SUBMITTING
    float velY; // REMOVE BEFORE SUBMITTING

    /**
    * @param x1 The x-coordinate of the lower-left corner.
    * @param y1 The y-coordinate of the lower-left corner.
    * @param width The x-coordinate of the upper-right corner.
    * @param height The y-coordinate of the upper-right corner.
    */
    RTreeObject(float x, float y, float width, float height);
    
    void setPosition(float x, float y);

    // REMOVE BEFORE SUBMITTING
    void update(float mapWidth, float mapHeight);

    // REMOVE BEFORE SUBMITTING
    std::string print();
    
    // REMOVE BEFORE SUBMITTING
    void draw(const std::shared_ptr<SpriteBatch>& batch);
};

#endif
