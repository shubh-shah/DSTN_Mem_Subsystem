#include "main_memory.h"
#include <pthread.h>
#include <malloc.h>

extern void init_main_memory(main_memory* main_mem){
    main_mem->nr_free_frames = NUM_FRAMES;
    for(int i=0;i<NUM_FRAMES;i++){
        main_mem->frame_table[i].valid = 0;
    }
    main_mem->lru=NULL;
}

/*
    Input:
        Page No. (23 bits)
    Returns:
        Successful 0
        Invalid Refrence
        Page Fault
*/
int do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address){
    //RD WR PERMISSION CHECK?
    if(pgd_index(linear_address) >= task->ptlr)           //Doubt??
        return INVALID_REF;            //Invalid Reference

    uint32_t* pgd_ent = pgd_entry(task, linear_address);
    if(!is_valid_entry(*pgd_ent)){
        do_page_fault(main_mem, task, pgd_ent, linear_address, 2);
        return PAGE_FAULT;
    }

    uint32_t* pmd_ent = pmd_entry(main_mem->mem_arr, *pgd_ent, linear_address);
    if(!is_valid_entry(*pmd_ent)){
        do_page_fault(main_mem, task, pmd_ent, linear_address, 2);
        return PAGE_FAULT;
    }

    uint32_t* pld_ent = pld_entry(main_mem->mem_arr, *pmd_ent, linear_address);
    if(!is_valid_entry(*pld_ent)){
        do_page_fault(main_mem, task, pld_ent, linear_address, 1);
        return PAGE_FAULT;
    }
    
    uint32_t* pt_ent = pt_entry(main_mem->mem_arr, *pld_ent, linear_address);
    if(!is_valid_entry(*pt_ent)){
        do_page_fault(main_mem, task, pt_ent, linear_address, 0);
        return PAGE_FAULT;
    }
    //Shift to tail of LRU Queue
    frame_table_entry* curr_node = main_mem->lru;
    if(curr_node == NULL){
        //Shouldn't really occur. Just as a precaution
        printf("ERRORR!\n");
    }
    do{
        if(curr_node->page_table_entry == pt_ent){
            if(curr_node==main_mem->lru){
                if(curr_node->lru_next == curr_node){
                    goto shiftDone;
                }
                main_mem->lru = main_mem->lru->lru_next;
            }
            frame_table_entry* next = curr_node->lru_next;
            frame_table_entry* prev = curr_node->lru_prev;
            prev->lru_next = next;
            next->lru_prev = prev;
            break;
        }
        curr_node = curr_node->lru_next;
    }while(curr_node != main_mem->lru);
    // if(couldn't find'){
        //Shouldn't happen
    // }
    frame_table_entry* tail = main_mem->lru->lru_prev;
    tail->lru_next = curr_node;
    curr_node->lru_prev = tail;
    main_mem->lru->lru_prev = curr_node;
    curr_node->lru_next = main_mem->lru->lru_prev;
shiftDone:
    insert_tlb_entry(tlb, task, linear_address, *pt_ent);
    return 0;
}

/*
    This routine handles page faults.  
    It creates a different thread, passes all arguments to it and returns
 */
struct pass_pg_fault{
    main_memory* main_mem;
    uint32_t* invalid_entry;
    uint32_t linear_address;
    bool is_pgtbl;
    task_struct* task;
};
//Synchronistaion problems??? Lock a page table????
void* _do_page_fault(void* args){ 
    uint32_t linear_address = ((struct pass_pg_fault*) args)->linear_address;
    uint32_t* entry = ((struct pass_pg_fault*) args)->invalid_entry;
    bool is_pgtbl = ((struct pass_pg_fault*) args)->is_pgtbl;
    main_memory* main_mem = ((struct pass_pg_fault*) args)->main_mem;
    task_struct* task = ((struct pass_pg_fault*) args)->task;
    free((struct pass_pg_fault*) args);
    if(is_valid_entry(*entry)){
        return NULL;
    }
    uint32_t frame_no = get_zeroed_page(main_mem, task, entry, is_pgtbl);
    // if(!is_valid_frame_no(frame_no)){
    //     //Decide
    //     printf("Page Fault : Couldn't allocate a frame");
    //     return NULL;
    // }
    //Set the entry:
    *entry = frame_no|VALID_MASK;
    if(!is_pgtbl){ //To do this need to handle replacement as well - If i don't do this, have to update lru for pg table access
        swap_in(main_mem, frame_no, linear_address ,task);
    }
    return NULL;
}
void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl){
    if(is_valid_entry(*invalid_entry)){
        return;
    }
    //Change status of task to waiting
    pthread_t tid_listen;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    struct pass_pg_fault* args = malloc(sizeof(struct pass_pg_fault))   ;
    args->invalid_entry = invalid_entry;
    args->linear_address = linear_address;
    args->is_pgtbl = is_pgtbl;
    args->main_mem = main_mem;
    args->task = task;
    pthread_create(&tid_listen,&attr,_do_page_fault,(void*)args);
}

uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl){
    frame_table_entry* frtbl_entry;
    if(main_mem->nr_free_frames == 0){     
        if (task->frames_used < MAX_FRAMES_PER_TASK) {
            frtbl_entry = main_mem->lru;
            //Invalidate pg table and frame table
            invalidate_entry(fr_entry);
        }
        else{
            replace_a_local_frame;
        }
    }else{

    }
    //Should i add page table frames tot lru queues???
    for(uint32_t i=0;i<NUM_FRAMES;i++){
        if(!main_mem->frame_table[i].valid){
            //Update Page Table Entry

            //Update Frame Table Entry
            main_mem->frame_table[i].page_table_entry = pgtbl_entry;
            main_mem->frame_table[i].pid = task->pid;
            main_mem->frame_table[i].valid = 1;
            //Update LRU Queue
            if(!is_pgtbl){
                main_mem->frame_table[i].lru_prev = ;
                main_mem->frame_table[i].lru_next = ;
                main_mem->lru->lru_prev=


                if(main_mem->lru==NULL){
                    main_mem->lru
                }
            }
            //Update Global Counters
            main_mem->nr_free_frames--;
            task->frames_used++;
            return i;
        }
    }
    //Zero out the frame
}

// How to handle replacement for page tables?????
// Working set strategy - Count page table accesses in locality?
// Do we have to implement malloc like behaviour?
// Read write - both needed and if yes how to determine from trace?
// Include problems-everyone needs something of mem_strct, make gloabal or smthing?