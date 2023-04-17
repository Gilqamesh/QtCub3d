#include "camera.h"

Camera::Camera(r32 x, r32 y, r32 phi)
    : p(x, y)
{
    this->phi = phi;

    update();
}

void Camera::updateDirection(r32 dphi) {
    this->phi += dphi;

    update();
}

void Camera::updatePosition(v2<r32> dp) {
    p += dp;
}

void Camera::update(void) {
    dir.x = cos(phi);
    dir.y = sin(phi);
    plane.x = -sin(phi);
    plane.y = cos(phi);
}
