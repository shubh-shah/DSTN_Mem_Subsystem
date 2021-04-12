#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include "../task.h"
#include "../cache/tlb.h"

// #define TASK_SIZE -Is size of a tak to be limited?
#define MAX_FRAMES_PER_TASK ???
#define MIN_FRAMES_PER_TASK ???

//ERROR CODES:
#define INVALID_REF 1
#define PAGE_FAULT 2

typedef struct{
    frame_table_entry frame_table[NUM_FRAMES];
    uint8_t mem_arr[MM_SIZE];
    int nr_free_frames;
    frame_table_entry*  lru;
    // extern unsigned long free_page_list;
}main_memory;


//In main_mem.c
extern void init_main_memory(main_memory* main_mem);
extern int do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address);
extern void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl);
extern uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl);

//In swap.c
extern void swap_in(main_memory* main_mem, task_struct* task, uint32_t frame_no, uint32_t linear_address);
extern void swap_out(main_memory* main_mem, task_struct* task, uint32_t frame_no, uint32_t linear_address);

// extern int nr_swap_pages;

//Write or shared pages????

#endif