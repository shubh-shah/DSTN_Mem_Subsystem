#ifndef L1_CACHE_H
#define L1_CACHE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "l2_cache.h"

#define L1_OFFSET_BITS 4
#define L1_INDEX_BITS 6
#define L1_TAG_BITS 15
#define L1_INDEX_SHIFT L1_OFFSET_BITS
#define L1_TAG_SHIFT (L1_OFFSET_BITS + L1_INDEX_BITS)

#define L1_OFFSET_MASK 0xf
#define L1_INDEX_MASK 0x3f
#define L1_TAG_MASK 0x7fff

#define L1_SET_SIZE 4
#define L1_NUM_SETS 64


typedef struct {
    uint16_t tags[L1_SET_SIZE];
    bool valid[L1_SET_SIZE];
    bool matrix[L1_SET_SIZE][L1_SET_SIZE];
} l1_cache_set;

typedef struct {
    l1_cache_set sets[L1_NUM_SETS];
} l1_cache;


extern l1_cache *init_l1_cache();

void mark_recency(bool matrix[L1_SET_SIZE][L1_SET_SIZE], int i);

int get_block_to_replace(bool matrix[L1_SET_SIZE][L1_SET_SIZE]);

extern bool read_l1_cache(l1_cache *cache, uint32_t physical_address);

extern bool write_l1_cache(l1_cache *l1, uint32_t physical_address);

extern bool update_l1_cache(l1_cache *l1, uint32_t physical_address);

#endif