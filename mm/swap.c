#include "main_memory.h"

//Initial state of disk map?
disk_map_entry* init_disk_map_entry(int pid, uint32_t* page_table_entry, int loc){
    disk_map_entry* new = malloc(sizeof(disk_map_entry));
    new->pid = pid;
    new->page_table_entry = page_table_entry;
    new->location_in_sec_mem = loc;
}

bool swap_in(main_memory* main_mem, task_struct* task, uint32_t* page_table_entry){
    if (isEmpty(main_mem->disk_map))
        return 1;         /* Page not in disk, Means dirty */
    q_node* curr = main_mem->disk_map->front;
    q_node* prev = NULL;
    while( (((disk_map_entry*)(curr->data_ptr))->page_table_entry != page_table_entry) && (((disk_map_entry*)(curr->data_ptr))->pid != task->pid) ) {
        if(curr->next == NULL)
            return 1;     /* Page not in disk, Means dirty */
        prev = curr;
        curr = curr->next;
    }
    uint64_t sec_base_address = ((disk_map_entry*)(curr->data_ptr))->location_in_sec_mem;
    /* Can Iterate and get the data from secondary memory using this address */
    return 0;
}

void swap_out(main_memory* main_mem, frame_table_entry* frame_entry){
    if (isEmpty(main_mem->disk_map)){
        /* Get free block from seconday memory and place it's address here*/
        /* Move the frame to this block */
        uint64_t sec_free_blk_base_address;
        push(main_mem->disk_map,init_disk_map_entry(frame_entry->pid, frame_entry->page_table_entry, sec_free_blk_base_address));
        return;
    }
    q_node* curr = main_mem->disk_map->front;
    q_node* prev = NULL;
    while( (((disk_map_entry*)(curr->data_ptr))->page_table_entry != frame_entry->page_table_entry) && (((disk_map_entry*)(curr->data_ptr))->pid != frame_entry->pid) ) {
        if(curr->next == NULL){
            /* Get free block from seconday memory and place it's address here*/
            /* Move the frame to this block */
            uint64_t sec_free_blk_base_address;
            push(main_mem->disk_map,init_disk_map_entry(frame_entry->pid, frame_entry->page_table_entry, sec_free_blk_base_address));
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    uint64_t sec_base_address = ((disk_map_entry*)(curr->data_ptr))->location_in_sec_mem;
    /* Move the frame to this block */
}