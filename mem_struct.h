#ifndef MEM_STRUCT_H
#define MEM_STRUCT_H

#include <stdint.h>
#include "mm/main_memory.h"
#include "cache/tlb.h"

typedef struct{
    trans_look_buff* tlb;
    // l1_cache l1cache;
    // l2_cache l2cache;

    main_memory* main_mem;
    // sec_memory sec_mem;

    uint32_t reg;//make this byte sized?
} memory_subsystem;

#endif