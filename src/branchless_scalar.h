#ifndef BRANCHLESS_SCALAR_H
#define BRANCHLESS_SCALAR_H

#include <stdint.h>

/* Portable branchless scalar operations - compiles to cmov/csel on modern architectures */

/* Conditional select: cond ? a : b using arithmetic */
static inline int32_t bl_select_i32(int32_t cond, int32_t a, int32_t b) {
    int32_t mask = -(cond != 0);
    return (a & mask) | (b & ~mask);
}

static inline int64_t bl_select_i64(int64_t cond, int64_t a, int64_t b) {
    int64_t mask = -(cond != 0);
    return (a & mask) | (b & ~mask);
}

/* Branchless absolute value */
static inline int32_t bl_abs_i32(int32_t x) {
    int32_t mask = x >> 31;
    return (x ^ mask) - mask;
}

static inline int64_t bl_abs_i64(int64_t x) {
    int64_t mask = x >> 63;
    return (x ^ mask) - mask;
}

/* Branchless min */
static inline int32_t bl_min_i32(int32_t a, int32_t b) {
    return b ^ ((a ^ b) & -(a < b));
}

static inline int64_t bl_min_i64(int64_t a, int64_t b) {
    return b ^ ((a ^ b) & -(a < b));
}

static inline uint32_t bl_min_u32(uint32_t a, uint32_t b) {
    return b ^ ((a ^ b) & -(a < b));
}

static inline uint64_t bl_min_u64(uint64_t a, uint64_t b) {
    return b ^ ((a ^ b) & -(a < b));
}

/* Branchless max */
static inline int32_t bl_max_i32(int32_t a, int32_t b) {
    return a ^ ((a ^ b) & -(a < b));
}

static inline int64_t bl_max_i64(int64_t a, int64_t b) {
    return a ^ ((a ^ b) & -(a < b));
}

static inline uint32_t bl_max_u32(uint32_t a, uint32_t b) {
    return a ^ ((a ^ b) & -(a < b));
}

static inline uint64_t bl_max_u64(uint64_t a, uint64_t b) {
    return a ^ ((a ^ b) & -(a < b));
}

/* Branchless clamp */
static inline int32_t bl_clamp_i32(int32_t x, int32_t lo, int32_t hi) {
    x = bl_max_i32(x, lo);
    return bl_min_i32(x, hi);
}

static inline int64_t bl_clamp_i64(int64_t x, int64_t lo, int64_t hi) {
    x = bl_max_i64(x, lo);
    return bl_min_i64(x, hi);
}

/* Branchless sign: returns -1, 0, or 1 */
static inline int32_t bl_sign_i32(int32_t x) {
    return (x >> 31) | ((-x) >> 31);
}

static inline int64_t bl_sign_i64(int64_t x) {
    return (x >> 63) | ((-x) >> 63);
}

/* Force non-negative (clamp to zero) */
static inline int32_t bl_force_non_negative_i32(int32_t x) {
    int32_t mask = x >> 31;
    return x & ~mask;
}

static inline int64_t bl_force_non_negative_i64(int64_t x) {
    int64_t mask = x >> 63;
    return x & ~mask;
}

/* Conditional add: a + (cond ? b : 0) */
static inline int32_t bl_conditional_add_i32(int32_t a, int32_t b, int32_t cond) {
    int32_t mask = -(cond != 0);
    return a + (b & mask);
}

static inline int64_t bl_conditional_add_i64(int64_t a, int64_t b, int64_t cond) {
    int64_t mask = -(cond != 0);
    return a + (b & mask);
}

/* Branchless boolean operations */
static inline int32_t bl_bool_and(int32_t a, int32_t b) {
    return (a != 0) & (b != 0);
}

static inline int32_t bl_bool_or(int32_t a, int32_t b) {
    return (a != 0) | (b != 0);
}

static inline int32_t bl_bool_not(int32_t a) {
    return (a == 0);
}

/* Branchless comparison results (-1 for true, 0 for false) */
static inline int32_t bl_cmp_eq_i32(int32_t a, int32_t b) {
    return -(a == b);
}

static inline int32_t bl_cmp_ne_i32(int32_t a, int32_t b) {
    return -(a != b);
}

static inline int32_t bl_cmp_lt_i32(int32_t a, int32_t b) {
    return -(a < b);
}

static inline int32_t bl_cmp_le_i32(int32_t a, int32_t b) {
    return -(a <= b);
}

static inline int32_t bl_cmp_gt_i32(int32_t a, int32_t b) {
    return -(a > b);
}

static inline int32_t bl_cmp_ge_i32(int32_t a, int32_t b) {
    return -(a >= b);
}

#endif /* BRANCHLESS_SCALAR_H */
