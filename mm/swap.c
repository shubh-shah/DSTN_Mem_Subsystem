#include "main_memory.h"

//Initial state of disk map?
disk_map_entry* init_disk_map_entry(int pid, uint32_t* page_table_entry, int loc){
    disk_map_entry* new = malloc(sizeof(disk_map_entry));
    new->pid = pid;
    new->page_table_entry = page_table_entry;
    new->location_in_sec_mem = loc;
    return new;
}

/* 
    Swap in a page  into frame_number pointed by page_table_entry 
    Returns: Success - 0, Failure - 1
*/
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

/* 
    Swap a page indicated by frame_entry out of main memory. 
    Global and clean pages are not swapped out
    Resets the DIRTY bit
*/
void swap_out(main_memory* main_mem, frame_table_entry* frame_entry){
    if((*(frame_entry->page_table_entry))&GLOBAL_MASK){   /* If Global, Don't swap out */
        return;
    }
    if((*(frame_entry->page_table_entry))&DIRTY_MASK){    /* If Dirty, only then swap out */
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
        *(frame_entry->page_table_entry) = reset_bit_pgtbl_entry((*(frame_entry->page_table_entry)),DIRTY_MASK);
    }
}

/*
    Suspend or Shut a process down.
    If suspend, then removes all the pages from main memory, invalidates entries in page tables and deallocates the frames.
    Shudown also deallocates all page tables.
    Changes status of task to SWAPPED_OUT or FINISHED based on the suspend argument
*/
void unload_task(main_memory* main_mem, task_struct* task, bool suspend){
    if(suspend){
        /* Page walk and swap out pages */
        for(uint32_t page_no=0;page_no<=UINT32_MAX/PG_SIZE;){
            uint32_t linear_address = page_no*PG_SIZE;
            if(pgd_index(linear_address) >= task->ptlr)      
                break;

            uint32_t* pgd_ent = pgd_entry(task, linear_address);
            if(!is_valid_entry(*pgd_ent)){
                linear_address += ENTRY_PER_PG*ENTRY_PER_PG*ENTRY_PER_PG;
            }

            uint32_t* pmd_ent = pmd_entry(gm_subsys->main_mem->mem_arr, *pgd_ent, linear_address);
            if(!is_valid_entry(*pmd_ent)){
                linear_address += ENTRY_PER_PG*ENTRY_PER_PG;
            }

            uint32_t* pld_ent = pld_entry(gm_subsys->main_mem->mem_arr, *pmd_ent, linear_address);
            if(!is_valid_entry(*pld_ent)){
                page_no += ENTRY_PER_PG; 
                continue;
            }
            
            uint32_t* pt_ent = pt_entry(gm_subsys->main_mem->mem_arr, *pld_ent, linear_address);
            page_no++;
            if(!is_valid_entry(*pt_ent)){
                continue;
            }
            *pt_ent = reset_bit_pgtbl_entry(*pt_ent,VALID_MASK);
            frame_table_entry* frame_entry = page_table_entry_to_frame_table_entry_ptr(main_mem->frame_tbl->table,*pt_ent);
            swap_out(main_mem, frame_entry);
            /* If frame not global, unallocate corresponding frames */
            if(frame_entry->pid!=-1){
                frame_entry->valid = 0;
            }
        }
    }
    else{
        /* If finishing, unallocate every frame allocated, including page tables */
        for(int i=0;i<NUM_FRAMES;i++){
            if(main_mem->frame_tbl->table[i].pid==task->pid){
                main_mem->frame_tbl->table[i].valid = 0;
            }
        }
    }
    /* Clear the LRU Queue of the frames from this process */
    while(lru_remove_by_pid(main_mem->frame_tbl, task->pid)!=NULL);
    
    if(suspend){
        task->status = SWAPPED_OUT;
    }
    else{
        task->status = FINISHED;
    }
}