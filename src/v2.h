
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
v2<T> &operator*=(v2<T> &v, T s) {
    v.x *= s;
    v.y *= s;

    return v;
}

template <typename T>
v2<T> operator*(v2<T> v, T s) {
    v *= s;

    return v;
}

template <typename T>
v2<T> operator*(T s, v2<T> v) {
    v *= s;

    return v;
}

template <typename T>
v2<T> hadamard_product(v2<T> v, v2<T> w) {
    v.x *= w.x;
    v.y *= w.y;
    
    return v;
}

template <typename T>
v2<T> v2_normalize(v2<T> v) {
    return v * (static_cast<T>(1.0) / v2_length(v));
}

template <typename T>
v2<T> &operator+=(v2<T> &v, v2<T> w) {
    v.x += w.x;
    v.y += w.y;

    return v;
}

template <typename T>
v2<T> operator+(v2<T> v, v2<T> w) {
    v += w;

    return v;
}

template <typename T>
v2<T> operator-(v2<T> v) {
    v *= static_cast<T>(-1.0);
    return v;
}

template <typename T>
v2<T> operator-(v2<T> v, v2<T> w) {
    v.x -= w.x;
    v.y -= w.y;

    return v;
}

template <typename T>
v2<T> operator/(v2<T> v, T x) {
    v.x /= x;
    v.y /= x;

    return v;
}

#endif // V2_H
