#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include "../task.h"
#include "../tlb/tlb.h"

/* Thrashing Prevention */
#define MAX_FRAMES_PER_TASK (24*1024)       /* 24K */
#define MIN_FRAMES_PER_TASK 16
#define WORKING_SET_WINDOW 500000            /* microseconds */
#define TIMER_INTERVAL 50000                 /* microiseconds */
#define WORKING_SET_BITS 10
#define LOWER_LIMIT_THRASHING ((NUM_FRAMES*3)/4)
#define UPPER_LIMIT_THRASHING NUM_FRAMES

/* ERROR CODES: */
#define INVALID_REF 1
#define PAGE_FAULT 2

typedef struct{
    uint8_t mem_arr[MM_SIZE];
    frame_table* frame_tbl;
    int nr_free_frames;
    queue* disk_map; 
    bool thrashing;
} main_memory;


/* 
In main_mem.c 
Refer to paging.h for details on memory structure.
*/
extern main_memory* init_main_memory();
extern uint32_t do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address);
extern void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl);
extern uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl);
extern uint32_t get_global_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl);
extern void working_set_interrupt_handler(int sig);
extern void deallocate_frame(main_memory* main_mem, frame_table_entry* entry);

/* For transfers to and from caches */
extern void read_main_memory(main_memory* main_mem, uint32_t physical_address);
extern void write_main_memory(main_memory* main_mem, trans_look_buff* tlb, uint32_t physical_address);
/*
In swap.c 
Swap Behaviour:
    If a page not found in swap: marked as dirty on initialisation.
    Else marked clean.
    On any modifications, marked dirty.
    Marked clean after swap out.
*/
#define DISK_MAP_SIZE (1024*1024) /* Essentially limits size of virtual space */
typedef struct
{
    int pid;
    uint32_t* page_table_entry;     /* This works because page tables are never swapped out */
    uint64_t location_in_sec_mem;  
} disk_map_entry;

extern bool swap_in(main_memory* main_mem, task_struct* task, uint32_t* page_table_entry);
extern void swap_out(main_memory* main_mem, task_struct* task, uint32_t* page_table_entry);
extern void unload_task(main_memory* main_mem, task_struct* task, bool suspend);

#endif