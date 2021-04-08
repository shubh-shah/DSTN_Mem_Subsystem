#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include <stdbool.h>  

#include "task.h"

typedef struct{

}trans_look_buff;

extern void   init_tlb(trans_look_buff* tlb);
//task struct bcuz need pid
extern uint32_t get_frame_no_tlb(trans_look_buff* tlb, task_struct* task, uint32_t linear_address);
extern void insert_tlb_entry(trans_look_buff* tlb, task_struct* task, uint32_t linear_address, uint32_t page_tbl_entry);

#endif