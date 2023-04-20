#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

#include <cstdlib>

static inline int bitCount(i32 x) {
    if (x == 0) {
        return 0;
    }
    return 1 + bitCount(x & (x - 1));
}

static inline bool isPowOf2(i32 x) {
    return bitCount(x) == 1;
}

static inline void* myaligned_alloc(u32 alignment, u32 size) {
    void* res;
#if defined(_MSC_VER)
    res = _aligned_malloc(size, 64);
#else
    res = std::aligned_alloc(64, size);
#endif

    if (res == nullptr) {
        throw std::runtime_error("failed to allocate " + std::to_string(size) + " bytes");
    }

    return res;
}

static inline void myaligned_free(void* data) {
    if (data) {
#if defined(_MSC_VER)
        _aligned_free(data);
#else
        std::free(data);
#endif
    }
}

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

#endif
