#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include "task.h"
#include "tlb.h"

//ERROR CODES:
#define INVALID_REF 1
#define PAGE_FAULT 2

typedef struct{
    frame_table_entry frame_table[NUM_FRAMES];
    uint8_t mem_arr[MM_SIZE];
    int free_frames;
}main_memory;


//main_mem.c
extern void init_main_memory(main_memory* main_mem);

extern int do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address);

extern void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl);

extern uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task);





// extern int find_free_frame();



// extern char load_byte_mem(memory_subsystem mem, int physical_address);

// extern void store_byte_mem(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);

// extern char* retreive_block(int start_add,int length);

// extern void store_block(int start_add,int length, char* data);


//Useful stuff from linux kernel:


// extern int nr_swap_pages;
// extern int nr_free_pages;
// extern unsigned long free_page_list;
// extern inline unsigned long get_free_page(int priority);
// extern void free_page(unsigned long addr);



#endif