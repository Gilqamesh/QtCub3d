
#ifndef CLAMP_H
#define CLAMP_H

#include "defs.h"

template <typename T>
inline T clamp_value(T min, T val, T max) {
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

#endif // CLAMP_H
