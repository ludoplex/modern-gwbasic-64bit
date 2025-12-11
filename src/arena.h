#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *base;
    size_t offset;
    size_t capacity;
} Arena;

/* Initialize arena with given capacity */
static inline Arena *arena_create(size_t capacity) {
    Arena *a = (Arena *)malloc(sizeof(Arena));
    a->base = (uint8_t *)malloc(capacity);
    a->offset = 0;
    a->capacity = capacity;
    return a;
}

/* Branchless arena allocation - returns NULL on overflow, no branches */
static inline void *arena_alloc(Arena *a, size_t size) {
    size_t new_offset = a->offset + size;
    /* Check for overflow: new_offset > capacity or wrapped around */
    size_t overflow = (new_offset > a->capacity) | (new_offset < a->offset);
    size_t overflow_mask = -(overflow != 0);
    
    void *ptr = a->base + a->offset;
    /* Update offset only when no overflow */
    a->offset = (new_offset & ~overflow_mask) | (a->offset & overflow_mask);
    
    /* Return NULL on overflow, ptr otherwise */
    return (void *)((uintptr_t)ptr & ~overflow_mask);
}

/* Branchless aligned allocation */
static inline void *arena_alloc_aligned(Arena *a, size_t size, size_t align) {
    /* Align offset up */
    size_t mask = align - 1;
    size_t aligned_offset = (a->offset + mask) & ~mask;
    size_t new_offset = aligned_offset + size;
    
    /* Check for overflow */
    size_t overflow = (new_offset > a->capacity) | (new_offset < aligned_offset);
    size_t overflow_mask = -(overflow != 0);
    
    void *ptr = a->base + aligned_offset;
    /* Update offset only when no overflow */
    a->offset = (new_offset & ~overflow_mask) | (a->offset & overflow_mask);
    
    return (void *)((uintptr_t)ptr & ~overflow_mask);
}

/* Reset arena for reuse */
static inline void arena_reset(Arena *a) {
    a->offset = 0;
}

/* Free arena */
static inline void arena_destroy(Arena *a) {
    free(a->base);
    free(a);
}

/* Get remaining capacity */
static inline size_t arena_remaining(Arena *a) {
    return a->capacity - a->offset;
}

/* Branchless string duplication in arena */
static inline char *arena_strdup(Arena *a, const char *str) {
    size_t len = strlen(str) + 1;
    char *copy = (char *)arena_alloc(a, len);
    size_t is_null = (copy == NULL);
    size_t should_copy = (is_null == 0);
    
    /* Only copy when allocation succeeded (branchless) */
    size_t i = 0;
    while (i < len) {
        copy[i] = (char)((unsigned char)str[i] & -(should_copy != 0));
        i += should_copy;
    }
    
    return copy;
}

#endif /* ARENA_H */
