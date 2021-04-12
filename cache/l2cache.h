#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t cache_arr[]
}l2cache;


extern bool load_store_l2cache(l2cache* l2_cache, uint32_t physical_address){
    
}