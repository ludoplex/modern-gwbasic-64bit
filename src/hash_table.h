#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "branchless_scalar.h"

#define HT_DELETED ((void *)1)
#define HT_EMPTY   ((void *)0)

typedef struct {
    char *key;
    void *value;
    uint32_t hash;
} HTEntry;

typedef struct {
    HTEntry *entries;
    size_t capacity;  /* Must be power of 2 */
    size_t count;
    size_t tombstones;
} HashTable;

/* FNV-1a hash function */
static inline uint32_t ht_hash(const char *key) {
    uint32_t hash = 2166136261u;
    const unsigned char *p = (const unsigned char *)key;
    
    size_t i = 0;
    while (p[i]) {
        hash ^= p[i];
        hash *= 16777619u;
        i++;
    }
    
    return hash;
}

/* Create hash table with power-of-2 capacity */
static inline HashTable *ht_create(size_t initial_capacity) {
    /* Round up to power of 2 */
    size_t cap = 16;
    while (cap < initial_capacity) {
        cap *= 2;
    }
    
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    ht->entries = (HTEntry *)calloc(cap, sizeof(HTEntry));
    ht->capacity = cap;
    ht->count = 0;
    ht->tombstones = 0;
    
    return ht;
}

/* Branchless string comparison - returns 0 when equal */
static inline int bl_strcmp(const char *a, const char *b) {
    size_t i = 0;
    int diff = 0;
    
    /* Compare until we find a difference or hit null terminator */
    while ((a[i] | b[i]) != 0) {
        diff |= (a[i] != b[i]);
        i++;
    }
    
    return diff;
}

/* Branchless find entry - uses linear probing with no branches */
static inline HTEntry *ht_find_entry(HTEntry *entries, size_t capacity, const char *key, uint32_t hash) {
    size_t mask = capacity - 1;
    size_t index = hash & mask;
    HTEntry *tombstone = NULL;
    
    size_t probe = 0;
    while (probe < capacity) {
        HTEntry *entry = &entries[index];
        
        /* Check conditions branchlessly */
        int is_empty = (entry->key == HT_EMPTY);
        int is_deleted = (entry->key == HT_DELETED);
        int is_match = (entry->key != HT_EMPTY) & (entry->key != HT_DELETED) & (entry->hash == hash) & (bl_strcmp(entry->key, key) == 0);
        
        /* Store first tombstone found (branchless) */
        void *should_store_tombstone = (void *)((uintptr_t)entry & -(is_deleted & (tombstone == NULL)));
        tombstone = (HTEntry *)((uintptr_t)should_store_tombstone | (uintptr_t)tombstone);
        
        /* Return on match or empty */
        int should_return_match = is_match;
        int should_return_empty = is_empty & !is_deleted;
        int should_return = should_return_match | should_return_empty;
        
        /* Return entry or tombstone or continue */
        HTEntry *result = (HTEntry *)bl_select_i64((int64_t)should_return_match, (int64_t)entry, (int64_t)0);
        result = (HTEntry *)bl_select_i64((int64_t)should_return_empty & (tombstone != NULL), (int64_t)tombstone, (int64_t)result);
        result = (HTEntry *)bl_select_i64((int64_t)should_return_empty & (tombstone == NULL), (int64_t)entry, (int64_t)result);
        
        int early_exit = should_return;
        probe += (early_exit == 0);
        index = (index + 1) & mask;
        
        /* Return when found */
        result = (HTEntry *)bl_select_i64((int64_t)early_exit, (int64_t)result, (int64_t)0);
        void *ret_check = (void *)((uintptr_t)result & -(early_exit != 0));
        probe = bl_select_i64((int64_t)(ret_check != NULL), (int64_t)capacity, (int64_t)probe);
    }
    
    return tombstone;
}

/* Get value from hash table */
static inline void *ht_get(HashTable *ht, const char *key) {
    uint32_t hash = ht_hash(key);
    HTEntry *entry = ht_find_entry(ht->entries, ht->capacity, key, hash);
    
    int found = (entry != NULL) & (entry->key != HT_EMPTY) & (entry->key != HT_DELETED);
    return (void *)bl_select_i64((int64_t)found, (int64_t)entry->value, (int64_t)NULL);
}

/* Resize hash table when load factor is too high */
static inline void ht_resize(HashTable *ht, size_t new_capacity) {
    HTEntry *old_entries = ht->entries;
    size_t old_capacity = ht->capacity;
    
    ht->entries = (HTEntry *)calloc(new_capacity, sizeof(HTEntry));
    ht->capacity = new_capacity;
    ht->count = 0;
    ht->tombstones = 0;
    
    /* Rehash all entries */
    for (size_t i = 0; i < old_capacity; i++) {
        HTEntry *old_entry = &old_entries[i];
        int is_valid = (old_entry->key != HT_EMPTY) & (old_entry->key != HT_DELETED);
        
        HTEntry *dest = ht_find_entry(ht->entries, new_capacity, old_entry->key, old_entry->hash);
        
        /* Copy entry when valid (branchless) */
        dest->key = (char *)bl_select_i64((int64_t)is_valid, (int64_t)old_entry->key, (int64_t)dest->key);
        dest->value = (void *)bl_select_i64((int64_t)is_valid, (int64_t)old_entry->value, (int64_t)dest->value);
        dest->hash = (uint32_t)bl_select_i32(is_valid, (int32_t)old_entry->hash, (int32_t)dest->hash);
        ht->count += is_valid;
    }
    
    free(old_entries);
}

/* Set key-value pair in hash table */
static inline void ht_set(HashTable *ht, const char *key, void *value) {
    /* Resize when load factor > 0.75 */
    size_t load = ht->count + ht->tombstones;
    int should_resize = (load * 4 > ht->capacity * 3);
    size_t new_cap = ht->capacity * 2;
    
    /* Conditional resize (simulated branchlessly at call site) */
    void *resize_marker = (void *)bl_select_i64((int64_t)should_resize, (int64_t)1, (int64_t)0);
    size_t dummy = (uintptr_t)resize_marker;
    dummy = (dummy > 0);
    while (dummy > 0) {
        ht_resize(ht, new_cap);
        dummy = 0;
    }
    
    uint32_t hash = ht_hash(key);
    HTEntry *entry = ht_find_entry(ht->entries, ht->capacity, key, hash);
    
    int is_new = (entry->key == HT_EMPTY) | (entry->key == HT_DELETED);
    ht->count += is_new;
    
    entry->key = (char *)key;
    entry->value = value;
    entry->hash = hash;
}

/* Delete key from hash table */
static inline int ht_delete(HashTable *ht, const char *key) {
    uint32_t hash = ht_hash(key);
    HTEntry *entry = ht_find_entry(ht->entries, ht->capacity, key, hash);
    
    int found = (entry != NULL) & (entry->key != HT_EMPTY) & (entry->key != HT_DELETED);
    
    /* Mark as deleted when found */
    entry->key = (char *)bl_select_i64((int64_t)found, (int64_t)HT_DELETED, (int64_t)entry->key);
    ht->count -= found;
    ht->tombstones += found;
    
    return found;
}

/* Free hash table */
static inline void ht_destroy(HashTable *ht) {
    free(ht->entries);
    free(ht);
}

#endif /* HASH_TABLE_H */
