#include "main_memory.h"
#include "../global_variables.h"

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
    if(curr == main_mem->disk_map->front){
        main_mem->disk_map->front = main_mem->disk_map->front->next;
    }
    else{
        prev->next = curr->next;
    }
    if(curr==main_mem->disk_map->rear){
        main_mem->disk_map->rear = prev;
    }
    main_mem->disk_map->node_count--;
    uint64_t sec_base_address = ((disk_map_entry*)(curr->data_ptr))->location_in_sec_mem;
    /* Can Iterate and get the data from secondary memory using this address */
    return 0;
}

/* 
    Swap a page indicated by frame_entry out of main memory. 
    Global and clean pages are not swapped out
    Resets the DIRTY bit
*/
void swap_out(main_memory* main_mem, task_struct* task, uint32_t* page_table_entry){
    if((*(page_table_entry))&GLOBAL_MASK){   /* If Global, Don't swap out */
        return;
    }
    if((*(page_table_entry))&DIRTY_MASK){    /* If Dirty, only then swap out */
        uint64_t sec_free_blk_base_address; /*Get a free block in secondary memory */
        push(main_mem->disk_map,init_disk_map_entry(task->pid, page_table_entry, sec_free_blk_base_address));
        /* Move the frame to this block */
        *(page_table_entry) = reset_bit_pgtbl_entry((*(page_table_entry)),DIRTY_MASK);
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
        for(uint32_t pgd_offset=0;pgd_offset<task->ptlr;pgd_offset++){
            uint32_t* pgd_ent = pgd_entry_from_offset(task, pgd_offset);
            if(!is_valid_entry(*pgd_ent)){
                continue;
            }
            for(uint32_t pmd_offset=0;pmd_offset<ENTRY_PER_PG;pmd_offset++){
                uint32_t* pmd_ent = pmd_entry_from_offset(gm_subsys->main_mem->mem_arr, *pgd_ent, pmd_offset);
                if(!is_valid_entry(*pmd_ent)){
                    continue;
                }
                for(uint32_t pld_offset=0;pld_offset<ENTRY_PER_PG;pld_offset++){
                    uint32_t* pld_ent = pld_entry_from_offset(gm_subsys->main_mem->mem_arr, *pmd_ent, pld_offset);
                    if(!is_valid_entry(*pld_ent)){
                        continue;
                    }
                    for(uint32_t pt_offset=0;pt_offset<ENTRY_PER_PG;pt_offset++){
                        uint32_t* pt_ent = pt_entry_from_offset(gm_subsys->main_mem->mem_arr, *pld_ent, pt_offset);
                        if(!is_valid_entry(*pt_ent)){
                            continue;
                        }
                        *pt_ent = reset_bit_pgtbl_entry(*pt_ent,VALID_MASK);
                        frame_table_entry* frame_entry = page_table_entry_to_frame_table_entry_ptr(main_mem->frame_tbl->table,*pt_ent);
                        swap_out(main_mem, task, frame_entry->page_table_entry);
                        /* If frame not global(handled by the function), unallocate corresponding frames */
                        deallocate_frame(main_mem, frame_entry);
                    }
                }
            }
        }
    }
    else{
        /* If finishing, unallocate every frame allocated, including page tables */
        for(int i=0;i<NUM_FRAMES;i++){
            if(main_mem->frame_tbl->table[i].pid==task->pid){
                deallocate_frame(main_mem, main_mem->frame_tbl->table+i);
            }
        }
    }
    tlb_invalidate(gm_subsys->tlb,task);
    
    if(suspend){
        task->status = SWAPPED_OUT;
    }
    else{
        /* Clear disk map if process is finished */
        if (!isEmpty(main_mem->disk_map)){
            q_node* curr = main_mem->disk_map->front;
            q_node* prev = NULL;
            while(true) {
                if(curr->next == NULL)
                    break;
                if(((disk_map_entry*)(curr->data_ptr))->pid == task->pid){
                    if(curr == main_mem->disk_map->front){
                        main_mem->disk_map->front = main_mem->disk_map->front->next;
                    }
                    else{
                        prev->next = curr->next;
                    }
                    if(curr==main_mem->disk_map->rear){
                        main_mem->disk_map->rear = prev;
                    }
                    main_mem->disk_map->node_count--;
                    curr = curr->next;
                }
                else{
                    prev = curr;
                    curr = curr->next;
                }
            }
        }
        task->status = FINISHED;
    }
}