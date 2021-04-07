#ifndef MEM_SUBSYS_H
#define MEM_SUBSYS_H

#include <stdint.h>
#include <stdbool.h>  

#include "task.h"
#include "mem_struct.h"

#include "cache/tlb.h"
// #include "cache/l1_cache.h"
// #include "cache/l2_cache.h"
#include "main_memory/main_memory.h"
// #include "sec_memory/sec_memory.h"

extern void init_memory_subsystem(memory_subsystem* mem);


#define load_byte(mem, task, linear_address) load_store_byte(mem, task, linear_address, true)
#define store_byte(mem, task, linear_address) load_store_byte(mem, task, linear_address, false)

extern bool load_store_byte(memory_subsystem* mem, task_struct* task, uint32_t linear_address, bool load);

#endif