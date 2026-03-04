#ifndef TYPES_H_
#define TYPES_H_

#include <inttypes.h>

/* Integer */
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Float */
typedef float  f32;
typedef double f64;

/* Bool */
typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

typedef struct vec2_s {
    union {
        f32 x, height;
    };
    union {
        f32 y, width;
    };
} vec2_t;
typedef vec2_t point_t;
typedef vec2_t frame_t;

#endif /* TYPES_H_ */
