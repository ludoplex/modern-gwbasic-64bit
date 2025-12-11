#ifndef BRANCHLESS_AARCH64_H
#define BRANCHLESS_AARCH64_H

#ifdef __aarch64__

#include <arm_neon.h>
#include <stdint.h>

/* Conditional select: cond ? a : b using NEON BSL */
static inline uint32x4_t bl_select_neon(uint32x4_t cond, uint32x4_t a, uint32x4_t b) {
    return vbslq_u32(cond, a, b);
}

/* Branchless max */
static inline uint32x4_t bl_max_neon_u32(uint32x4_t a, uint32x4_t b) {
    return vmaxq_u32(a, b);
}

static inline int32x4_t bl_max_neon_s32(int32x4_t a, int32x4_t b) {
    return vmaxq_s32(a, b);
}

/* Branchless min */
static inline uint32x4_t bl_min_neon_u32(uint32x4_t a, uint32x4_t b) {
    return vminq_u32(a, b);
}

static inline int32x4_t bl_min_neon_s32(int32x4_t a, int32x4_t b) {
    return vminq_s32(a, b);
}

/* Branchless absolute value */
static inline int32x4_t bl_abs_neon(int32x4_t x) {
    return vabsq_s32(x);
}

/* Branchless clamp */
static inline int32x4_t bl_clamp_neon(int32x4_t x, int32x4_t lo, int32x4_t hi) {
    x = bl_max_neon_s32(x, lo);
    return bl_min_neon_s32(x, hi);
}

/* Conditional add */
static inline int32x4_t bl_conditional_add_neon(int32x4_t a, int32x4_t b, uint32x4_t cond) {
    int32x4_t masked_b = vandq_s32(b, vreinterpretq_s32_u32(cond));
    return vaddq_s32(a, masked_b);
}

/* Scalar branchless operations for non-SIMD paths */
static inline int32_t bl_scalar_select(int32_t cond, int32_t a, int32_t b) {
    int32_t mask = -(cond != 0);
    return (a & mask) | (b & ~mask);
}

static inline int32_t bl_scalar_abs(int32_t x) {
    int32_t mask = x >> 31;
    return (x ^ mask) - mask;
}

static inline int32_t bl_scalar_min(int32_t a, int32_t b) {
    return b ^ ((a ^ b) & -(a < b));
}

static inline int32_t bl_scalar_max(int32_t a, int32_t b) {
    return a ^ ((a ^ b) & -(a < b));
}

static inline int32_t bl_scalar_clamp(int32_t x, int32_t lo, int32_t hi) {
    x = bl_scalar_max(x, lo);
    return bl_scalar_min(x, hi);
}

static inline int32_t bl_scalar_sign(int32_t x) {
    return (x >> 31) | ((-x) >> 31);
}

#endif /* __aarch64__ */

#endif /* BRANCHLESS_AARCH64_H */
