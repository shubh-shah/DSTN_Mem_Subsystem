#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include <stdbool.h>

#include "../task.h"
#include "../ADT/queue.h"
#include "../mm/paging.h"

#define NUM_BITS_PAGE 23
#define NUM_BITS_FRAME 16
#define NUM_TLB_ENTRIES 20
#define NUM_BITS_OFFSET 9

typedef struct {
    bool global;
    bool valid;
    uint8_t pid;
    uint32_t page_no: NUM_BITS_PAGE;
    uint32_t frame_no: NUM_BITS_FRAME;
    uint32_t page_table_entry;
} tlb_entry;

typedef queue trans_look_buff;

extern trans_look_buff *init_tlb();

extern uint32_t get_frame_no_tlb(trans_look_buff *tlb, task_struct *task, uint32_t linear_address);

extern void tlb_invalidate(trans_look_buff *tlb, task_struct *task);

extern void insert_tlb_entry(trans_look_buff *tlb, task_struct *task, uint32_t linear_address, uint32_t page_tbl_entry);

extern void set_dirty_bit_tlb(trans_look_buff *tlb, uint32_t frame_no);

extern void tlb_push(trans_look_buff *tlb, tlb_entry* entry);

#endif