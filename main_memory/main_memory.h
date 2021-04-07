#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"
#include "tlb.h"

#define MM_SIZE (32*1024*1024)          //32MB
#define PG_SIZE (512)                   //512B

//Page Table Constants
#define PG_TBL_ENTRY_SIZE 4
#define NUM_PG_TBL_ENTRIES_PER_PG (PG_SIZE/PG_TBL_ENTRY_SIZE)

//Frame Table Constants
#define FRAME_TBL_ENTRY_SIZE 4
#define NUM_FRAMES (MM_SIZE/PG_SIZE)    //64K

//ERROR CODES:
#define INVALID_REF (UINT32_MAX-1)
#define PAGE_FAULT UINT32_MAX

/*
    Memory Structure:
    Starting 10 Bytes for process details : PTBR, PTLR
    Bits required for logical address: 32 (2,7,7,7,9)
    Bits for offset: 9
    Bits to address a page:32-9=23
    Bits required for physical address: 25

    Next 4*64KB for Frame Table:
        FRAME TABLE Entry(4Bytes): Valid-1,PgNo-23,PID-8
        Pages required: 4*64K/512 = 4*64*2=512
    PAGE TABLE Entry(4Bytes): FrameNo:0-15,Valid-16,Global-17,Dirty-18,LRUbits:??,Caching-1,Protection-?? (12 bits left)
    PAGE DIR Entry(4Bytes): FrameNo-0-15,Valid-16,LRUbits:??,Caching-1,Protection-??
    //Do we want multiple size pages?
    //Free frame list??
*/

typedef struct{
    uint32_t page_number;
} frame_table_entry;

typedef struct{
    frame_table_entry frame_table[NUM_FRAMES];
    uint8_t mem_arr[MM_SIZE];
}main_memory;

#define is_bad_frame(frame_no) (frame_no >= NUM_FRAMES)

//Functions:

/*
    Create and init Frame Tables

*/

extern int do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t page_no);

extern void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t linear_address);






// extern void init_main_memory(memory_subsystem mem);

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