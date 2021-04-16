#include "tlb.h"

trans_look_buff *init_tlb() {
    trans_look_buff *tlb = createQueue(NUM_TLB_ENTRIES);
    return tlb;
}

uint32_t get_frame_no_tlb(trans_look_buff *tlb, task_struct *task, uint32_t linear_address) {

    uint32_t requested_page_no = linear_address >> NUM_BITS_OFFSET;
    q_node *node = tlb->front;
    tlb_entry *entry;

    for (int i = 0; i < tlb->node_count; i++) {
        entry = node->data_ptr;

        if (entry->page_no == requested_page_no && entry->valid) {
            // If a task pid is different from the entry, it means the page is shared
            if (entry->pid != task->pid)
                entry->global = true;

            return entry->frame_no;
        }

        node = node->next;
    }

    // TLB Miss
    return UINT32_MAX;
}

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

void insert_tlb_entry(trans_look_buff *tlb, task_struct *task, uint32_t linear_address, uint32_t page_tbl_entry) {

    tlb_entry *new_entry = (tlb_entry *) malloc(sizeof(tlb_entry));
    new_entry->valid = true;
    new_entry->pid = task->pid;
    new_entry->global = false;
    new_entry->page_no = linear_address >> NUM_BITS_OFFSET;
    new_entry->frame_no = page_tbl_entry >> PG_TBL_FRAME_SHIFT;

    push(tlb, new_entry);
}