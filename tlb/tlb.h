#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include <stdbool.h>  

#include "../task.h"

#define NUM_BITS_PAGE 23
#define NUM_BITS_FRAME 16
#define NUM_TLB_ENTRIES 20
#define NUM_BITS_OFFSET 9

typedef struct{
    uint32_t page_no:NUM_BITS_PAGE;
    uint8_t pid;
    bool valid;
    uint16_t frame_no:NUM_BITS_FRAME;
    uint8_t share_count;
}tlb_entry;

typedef struct{
    tlb_entry entries[20];
}trans_look_buff;

extern void   init_tlb(trans_look_buff* tlb);
//task struct bcuz need pid
extern uint32_t get_frame_no_tlb(trans_look_buff* tlb, task_struct* task, uint32_t linear_address);
extern void insert_tlb_entry(trans_look_buff* tlb, task_struct* task, uint32_t linear_address, uint32_t page_tbl_entry);

#endif