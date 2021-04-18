#ifndef MEM_STRUCT_H
#define MEM_STRUCT_H

#include <stdint.h>
#include "mm/main_memory.h"
#include "cache/l1_cache.h"
#include "cache/l2_cache.h"
#include "tlb/tlb.h"

typedef struct{
    trans_look_buff* tlb;
    l1_cache* l1cache;
    l2_cache* l2cache;
    main_memory* main_mem;
    uint32_t reg;
} memory_subsystem;

#endif