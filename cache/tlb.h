#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include <stdbool.h>  

#include "task.h"

typedef struct{

}trans_look_buff;

//task struct bcuz need pid
get_frame_no_tlb(trans_look_buff* tlb, task_struct* task, uint32_t page_no);
extern void insert_tlb_entry(trans_look_buff* tlb, task_struct* task, uint32_t page_no, uint32_t page_tbl_entry);

#endif