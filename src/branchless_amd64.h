#ifndef BRANCHLESS_AMD64_H
#define BRANCHLESS_AMD64_H

#ifdef __x86_64__

#include <immintrin.h>
#include <stdint.h>

/* Conditional select: cond ? a : b using AVX-512 ternary logic (0xCA = 11001010) */
static inline __m512i bl_select_m512i(__m512i cond, __m512i a, __m512i b) {
    return _mm512_ternarylogic_epi32(a, b, cond, 0xCA);
}

/* Branchless absolute value for 32-bit integers */
static inline __m512i bl_abs_epi32(__m512i x) {
    __m512i sign = _mm512_srai_epi32(x, 31);
    return _mm512_ternarylogic_epi32(x, _mm512_sub_epi32(_mm512_setzero_si512(), x), sign, 0xCA);
}

/* Branchless max(a, b) */
static inline __m512i bl_max_epi32(__m512i a, __m512i b) {
    __mmask16 cmp = _mm512_cmpgt_epi32_mask(a, b);
    return _mm512_mask_blend_epi32(cmp, b, a);
}

/* Branchless min(a, b) */
static inline __m512i bl_min_epi32(__m512i a, __m512i b) {
    __mmask16 cmp = _mm512_cmpgt_epi32_mask(a, b);
    return _mm512_mask_blend_epi32(cmp, a, b);
}

/* Branchless clamp(x, lo, hi) */
static inline __m512i bl_clamp_epi32(__m512i x, __m512i lo, __m512i hi) {
    x = bl_max_epi32(x, lo);
    return bl_min_epi32(x, hi);
}

/* Force non-negative - clamp to zero */
static inline __m512i bl_force_non_negative(__m512i x) {
    __m512i neg = _mm512_srai_epi32(x, 31);
    return _mm512_ternarylogic_epi32(_mm512_setzero_si512(), x, neg, 0xCA);
}

/* Conditional add: a + (cond ? b : 0) */
static inline __m512i bl_conditional_add(__m512i a, __m512i b, __m512i cond) {
    __m512i masked_b = _mm512_and_epi32(b, cond);
    return _mm512_add_epi32(a, masked_b);
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

#endif /* __x86_64__ */

#endif /* BRANCHLESS_AMD64_H */
