
#ifndef DEFS_H
#define DEFS_H

#include <cstdint>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;

typedef float  r32;
typedef double r64;

#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))

#include <iostream>
#include <iomanip>
#define LOG(x) (std::cout << x << std::endl)

#include <string>

#include <intrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

#include "v2.h"

#endif // DEFS_H
