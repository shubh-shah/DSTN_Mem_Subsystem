#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include "../task.h"
#include "../tlb/tlb.h"

/* Thrashing Prevention */
#define MAX_FRAMES_PER_TASK (24*1024)       //24K
#define MIN_FRAMES_PER_TASK 16
#define WORKING_SET_WINDOW 10000  //10000 microseconds???
#define TIMER_INTERVAL 1000 //1000 microiseconds
#define WORKING_SET_BITS 10

/* ERROR CODES: */
#define INVALID_REF 1
#define PAGE_FAULT 2

typedef struct{
    uint8_t mem_arr[MM_SIZE];
    frame_table* frame_tbl;
    int nr_free_frames;
    queue* disk_map  
    // frame_table_entry* free_frames_list;
} main_memory;


/* In main_mem.c */
extern main_memory* init_main_memory();
extern int do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address);
extern void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl);
extern uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl);
extern void working_set_interrupt_handler(int sig);
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
extern void swap_out(main_memory* main_mem, frame_table_entry* frame);

#endif