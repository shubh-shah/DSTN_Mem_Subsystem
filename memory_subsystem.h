#ifndef MEM_SUBSYS_H
#define MEM_SUBSYS_H

#include "cache/tlb.h"
// #include "cache/l1_cache.h"
// #include "cache/l2_cache.h"
#include "main_memory/main_memory.h"
// #include "sec_memory/sec_memory.h"

typedef struct{
    trans_lookaside_buff tlb;
    // l1_cache l1cache;
    // l2_cache l2cache;

    main_memory main_mem;
    // sec_memory sec_mem;
} memory_subsystem;

extern char load_byte(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address);

extern void store_byte(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);

#endif