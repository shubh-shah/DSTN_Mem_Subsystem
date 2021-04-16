#ifndef L2_CACHE_H
#define L2_CACHE_H

#include <stdint.h>
#include <stdbool.h>

# define L2_CACHE_SIZE (32*1024)

typedef struct {
    uint8_t cache_arr[L2_CACHE_SIZE];
}l2cache;


extern bool load_store_l2cache(l2cache* l2_cache, uint32_t physical_address);

#endif