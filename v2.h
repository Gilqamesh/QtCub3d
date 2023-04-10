
#ifndef V2_H
#define V2_H

#include "defs.h"

template <typename T>
struct v2 {
    v2() = default;
    v2(T x, T y) : x(x), y(y) {}

    T x;
    T y;
};

template <typename T>
T v2_dot(v2<T> v, v2<T> w) {
    return v.x * w.x + v.y * w.y;
}

template <typename T>
T v2_length(v2<T> v) {
    return sqrt(v2_dot(v, v));
}

template <typename T>
v2<T> v2_scale(v2<T> v, T s) {
    v.x *= s;
    v.y *= s;

    return v;
}

template <typename T>
v2<T> v2_normalize(v2<T> v) {
    return v2_scale(v, static_cast<T>(1.0) / v2_length(v));
}

#endif // V2_H
