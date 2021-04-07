#ifndef MEM_STRUCT_H
#define MEM_STRUCT_H

#include <stdint.h>

typedef struct{
    uint8_t* tlb;
    // l1_cache l1cache;
    // l2_cache l2cache;

    uint8_t* main_mem;
    // sec_memory sec_mem;

    uint32_t reg;//make this byte sized?
} memory_subsystem;

#endif