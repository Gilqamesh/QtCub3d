
#ifndef CAMERA_H
#define CAMERA_H

#include "defs.h"
#include "v2.h"

class Camera {
public:
    v2<r32> p;
    v2<r32> dir;
    v2<r32> plane;
    r32 phi;
public:
    Camera(r32 x, r32 y, r32 phi);
    void updateDirection(r32 dphi);
    void updatePosition(r32 position_dx, r32 position_dy);

private:
    void update(void);
};

#endif // CAMERA_H
