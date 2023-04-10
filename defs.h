
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
using namespace std;
#define LOG(x) (cout << x << endl)

#include <string>

#endif // DEFS_H
