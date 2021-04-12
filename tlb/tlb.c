#include "tlb.h"


uint32_t get_frame_no_tlb(trans_look_buff* tlb, task_struct* task, uint32_t linear_address){

    uint32_t page_number = linear_address >> NUM_BITS_OFFSET;
    for(int i=0; i < NUM_TLB_ENTRIES; i++){
        if(tlb->entries->pid == pid && tlb->entries->valid && tlb->entries->page_no == page_number) {
            // handle c
            return tlb->entries->frame_no;
        }
    }

    return UINT32_MAX;
}

void tlb_invalidate(tlb_buffer *tlb, unsigned int pid)
{
    for(int i=0; i<NO_OF_TLB_ENTRIES; i++)
    {
        if(tlb->entries[i].pid == pid)
            tlb->entries[i].valid = 0;
    }
}