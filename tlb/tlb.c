#include "tlb.h"
#include "../global_variables.h"

trans_look_buff *init_tlb() {
    trans_look_buff *tlb = createQueue(NUM_TLB_ENTRIES);
    return tlb;
}

/*
   Searches the TLB for the Frame Number corresponding to the Linear Address.
   Input:
       Pointer to the task accessing the tlb
       Linear Address
    Returns:
       Frame Number if TLB hit.
       UINT32_MAX if TLB miss.
*/
uint32_t get_frame_no_tlb(trans_look_buff *tlb, task_struct *task, uint32_t linear_address) {

    uint32_t requested_page_no = linear_address >> NUM_BITS_OFFSET;
    q_node *node = tlb->front;
    tlb_entry *entry;

    for (int i = 0; i < tlb->node_count; i++) {
        entry = node->data_ptr;

        if (entry->page_no == requested_page_no && (entry->pid == task->pid || entry->pid == -1) && entry->valid) {
            entry->page_table_entry = entry->page_table_entry | WORKING_SET_BIT;   /* Set working set bit */
            return entry->frame_no;
        }

        node = node->next;
    }

    task->stat.tlb_miss++;
    return UINT32_MAX;
}

/*
   Invalidates all the entries corresponding to the task which are not shared.
   Input:
       Pointer to the terminating task
*/
void tlb_invalidate(trans_look_buff *tlb, task_struct *task) {

    q_node *node = tlb->front;
    tlb_entry *entry;

    for (int i = 0; i < tlb->node_count; i++) {
        entry = node->data_ptr;

        if (!entry->global && entry->pid == task->pid)
            entry->valid = false;

        node = node->next;
    }
}

/*
   Adds a new entry at the front of the TLB.
   Input:
       Pointer to the task modifying the tlb
       Linear Address
       Page Table Entry
*/
void insert_tlb_entry(trans_look_buff *tlb, task_struct *task, uint32_t linear_address, uint32_t page_tbl_entry) {
    tlb_entry *new_entry = (tlb_entry *) malloc(sizeof(tlb_entry));
    new_entry->valid = true;
    new_entry->pid = task->pid;
    new_entry->global = page_tbl_entry & GLOBAL_MASK;
    if(new_entry->global){
        new_entry->pid = -1;
    }
    new_entry->page_no = linear_address >> NUM_BITS_OFFSET;
    new_entry->frame_no = page_tbl_entry & PAGE_MASK;
    new_entry->page_table_entry = page_tbl_entry;
    tlb_push(tlb, new_entry);
}

/*
   Sets the dirty bit in TLB, so that it gets written to main memory during replacement
   Input:
       Frame Number
*/
void set_dirty_bit_tlb(trans_look_buff *tlb, uint32_t frame_no){
    q_node *node = tlb->front;
    tlb_entry *entry;

    for (int i = 0; i < tlb->node_count; i++) {
        entry = node->data_ptr;
        if (entry->frame_no == frame_no && entry->valid) {
            entry->page_table_entry = entry->page_table_entry | DIRTY_MASK;
            return;
        }
        node = node->next;
    }
}

/* 
   If replacement occurs, write the page table entry back to main memory as working set and dirty bits might have been updated.
   Input:
       Page Table Entry
*/
void tlb_push(trans_look_buff *tlb, tlb_entry* entry){
    if (isFull(tlb)){
        q_node* node = pop(tlb);
        *(gm_subsys->main_mem->frame_tbl->table[((tlb_entry*)node->data_ptr)->frame_no].page_table_entry) =((tlb_entry*)node->data_ptr)->page_table_entry; 
    }
    push(tlb, entry);
}