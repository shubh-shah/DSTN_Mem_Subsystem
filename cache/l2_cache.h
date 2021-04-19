#ifndef L2_CACHE_H
#define L2_CACHE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../util/queue.h"
#include "../mm/main_memory.h"

#define L2_OFFSET_BITS 5
#define L2_INDEX_BITS 7
#define L2_TAG_BITS 13
#define L2_INDEX_SHIFT L2_OFFSET_BITS
#define L2_TAG_SHIFT (L2_OFFSET_BITS + L2_INDEX_BITS)

#define L2_OFFSET_MASK 0x1f
#define L2_INDEX_MASK 0x7f
#define L2_TAG_MASK 0x1fff

#define L2_SET_SIZE 8
#define L2_NUM_SETS 128


typedef struct {
    uint16_t tag: L2_TAG_BITS;
    uint8_t index: L2_INDEX_BITS;
    uint8_t offset: L2_OFFSET_BITS;
    uint8_t valid: 1;
    uint8_t dirty: 1;
} l2_cache_block;

typedef struct {
    queue *tags;
} l2_cache_set;

typedef struct {
    l2_cache_set sets[L2_NUM_SETS];
} l2_cache;


extern l2_cache *init_l2_cache();

void custom_push(queue *q, void *data, main_memory *mm);

void custom_pop(queue *q, main_memory *mm);

extern bool read_l2_cache(l2_cache *cache, uint32_t physical_address);

extern bool write_l2_cache(l2_cache *cache, uint32_t physical_address);

extern void update_l2_cache(l2_cache *cache, main_memory *mm, uint32_t physical_address);

#endif