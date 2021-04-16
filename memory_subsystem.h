#ifndef MEM_SUBSYS_H
#define MEM_SUBSYS_H

#include <stdint.h>
#include <stdbool.h>

#include "task.h"
#include "mem_struct.h"

#include "tlb/tlb.h"
// #include "cache/l1cache.h"
// #include "cache/l2cache.h"
#include "mm/main_memory.h"

extern memory_subsystem* init_memory_subsystem();
extern void init_task(int task_index, main_memory* mem);

#define load_byte(mem, task, linear_address) load_store_byte(mem, task, linear_address, true)
#define store_byte(mem, task, linear_address) load_store_byte(mem, task, linear_address, false)
extern bool load_store_byte(memory_subsystem* mem, task_struct* task, uint32_t linear_address, bool load);

#endif