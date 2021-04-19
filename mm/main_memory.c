#include "main_memory.h"
#include <pthread.h>
#include <malloc.h>
#include <sys/time.h>
#include <signal.h>
#include "../global_variables.h"

main_memory* init_main_memory(){
    main_memory* main_mem = malloc(sizeof(main_memory));
    main_mem->nr_free_frames = NUM_FRAMES;
    main_mem->frame_tbl = init_frame_table();
    main_mem->disk_map = createQueue(DISK_MAP_SIZE);
    main_mem->thrashing = 0;

    /* Set Timer for Working set */
    signal(SIGVTALRM,working_set_interrupt_handler);
    struct timeval interval;
    struct timeval zero;
    struct itimerval period;
    interval.tv_sec=0;
    interval.tv_usec=TIMER_INTERVAL;
    zero.tv_sec=0;
    zero.tv_usec=0;
    period.it_interval=zero;
    period.it_value=interval;
    setitimer(ITIMER_VIRTUAL,&period,NULL);
    return main_mem;
}

/*
Do page table walk and push the entry into TLB
Input:
    Linear Address (25 bits)
Returns:
    Successful 0
    Invalid Refrence
    Page Fault
*/
uint32_t do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address){
    /* Page table walk */
    if(pgd_index(linear_address) >= task->ptlr)           
        return INVALID_REF;

    uint32_t* pgd_ent = pgd_entry(task, linear_address);
    if(!is_valid_entry(*pgd_ent)){
        do_page_fault(main_mem, task, pgd_ent, linear_address, 1);
        return PAGE_FAULT;
    }

    uint32_t* pmd_ent = pmd_entry(main_mem->mem_arr, *pgd_ent, linear_address);
    if(!is_valid_entry(*pmd_ent)){
        do_page_fault(main_mem, task, pmd_ent, linear_address, 1);
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
    insert_tlb_entry(tlb, task, linear_address, *pt_ent);
    return 0;
}

/*
Page fault handling
*/
struct pass_pg_fault{
    main_memory* main_mem;
    uint32_t* invalid_entry;
    uint32_t linear_address;
    bool is_pgtbl;
    task_struct* task;
};
/* Seprate functions not _do_page_fault & do_page_fault really required, needed if using separate thread */
void* _do_page_fault(void* args){
    /* Get Arguments */
    uint32_t* entry = ((struct pass_pg_fault*) args)->invalid_entry;
    bool is_pgtbl = ((struct pass_pg_fault*) args)->is_pgtbl;
    main_memory* main_mem = ((struct pass_pg_fault*) args)->main_mem;
    task_struct* task = ((struct pass_pg_fault*) args)->task;
    free((struct pass_pg_fault*) args);
    /* Return directly if already valid */
    if(is_valid_entry(*entry)){
        return NULL;
    }

    uint32_t frame_no;
    /* Randomly decide if a page is global or not */
    int is_global = !(rand()%256);              /* Probability = 1/256 */
    if(is_global){
        frame_no = get_global_zeroed_page(main_mem, task, entry, is_pgtbl);
    }
    else{
        frame_no = get_zeroed_page(main_mem, task, entry, is_pgtbl);
    }
    /* Update page table entry */
    *entry = frame_no;
    if(!is_pgtbl){
        if(swap_in(main_mem, task, entry)){
            /* Page not in disk, Means mark dirty */
            *entry |= DIRTY_MASK;
        }
        else{
            /* Page fresh out of disk, means mark clean */
            *entry = reset_bit_pgtbl_entry((*entry),DIRTY_MASK);
        }
    }
    *entry |= VALID_MASK;
    if(is_global){
        *entry |= GLOBAL_MASK;
    }
    /* Update task status */
    task->status = READY;
    return NULL;
}
/*
This routine handles page faults.
*/
void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl){
    if(is_pgtbl){
        task->stat.page_fault_pt++;
    }
    else{
        task->stat.page_fault++;
    }
    if(is_valid_entry(*invalid_entry)){
        return;
    }
    task->status = WAITING;
    /* Set arguments */
    /* 
    pthread_t tid_listen;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    */
    struct pass_pg_fault* args = malloc(sizeof(struct pass_pg_fault));
    args->invalid_entry = invalid_entry;
    args->linear_address = linear_address;
    args->is_pgtbl = is_pgtbl;
    args->main_mem = main_mem;
    args->task = task;
    /* 
    pthread_create(&tid_listen,&attr,_do_page_fault,(void*)args);
    */
    _do_page_fault((void*)args);
}

/* 
If frame not global, deallocate corresponding frame and Invalidate the page
*/
void deallocate_frame(main_memory* main_mem, frame_table_entry* entry){
    if(entry->pid!=-1 && !((*(entry->page_table_entry))&GLOBAL_MASK)){
        /* Update Frame Table Entry */
        entry->valid = 0;
        /* Update Page Table Entry */
        (*(entry->page_table_entry)) = reset_bit_pgtbl_entry(*(entry->page_table_entry),VALID_MASK);
        /* Update LRU Queue */
        lru_remove_by_frame_tbl_entry(main_mem->frame_tbl, entry);
        /* Update Global Counters */
        main_mem->nr_free_frames++;
        find_task(gtasks, entry->pid)->frames_used--;
    }
}

/* 
Allocate a frame to a process.
If all frames occupied, LRU replacement.
*/
uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl){
    frame_table_entry* lru_entry;
    if(main_mem->nr_free_frames == 0){
        /* Replacement */ 
        if (task->frames_used < MAX_FRAMES_PER_TASK) {
            /* Global replacement */
            lru_entry = (frame_table_entry*)(pop(main_mem->frame_tbl->lru)->data_ptr);
        }
        else{
            /* If no. of frames allowed exceeded, local replacement */
            lru_entry = lru_remove_by_pid(main_mem->frame_tbl, task->pid);
        }
        task->stat.page_replacements++;
        swap_out(main_mem, task, lru_entry->page_table_entry);
        deallocate_frame(main_mem, lru_entry);
    }else{
        /* Get a free frame */
        for(int i=0;i<NUM_FRAMES;i++){
            if(!main_mem->frame_tbl->table[i].valid){
                lru_entry = (main_mem->frame_tbl->table)+i;
                break;
            }
        }
    }
    /* Update Frame Table Entry */
    lru_entry->page_table_entry = pgtbl_entry;
    lru_entry->pid = task->pid;
    lru_entry->valid = 1;
    /* Update LRU Queue */
    if(!is_pgtbl){
        push(main_mem->frame_tbl->lru, lru_entry);
    }
    /* Update Global Counters */
    main_mem->nr_free_frames--;
    task->frames_used++;
    /* Zero out the frame */
    uint32_t frame_no = lru_entry-main_mem->frame_tbl->table;
    for(int i=0;i<PG_SIZE;i++){
        main_mem->mem_arr[(frame_no<<PT_SHIFT)+i] = 0;
    }
    return frame_no;
}
/* 
Allocate a global frame to a process.
If all frames occupied, LRU replacement.
*/
uint32_t get_global_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl){
    uint32_t frame_no = get_zeroed_page(main_mem, task, pgtbl_entry, is_pgtbl);
    lru_remove_by_frame_tbl_entry(main_mem->frame_tbl, main_mem->frame_tbl->table+frame_no);    /* Remove from lru queue to make it non replaceable */
    main_mem->frame_tbl->table[frame_no].pid = -1;
    *pgtbl_entry|=GLOBAL_MASK;
    return frame_no;
}

/* 
Read a block from main memory:
Error checking not required as it is gauranteed the page is in memory at this point 
*/
void read_main_memory(main_memory* main_mem, uint32_t physical_address){
    /* Move entry to back of LRU Queue (Make MRU frame) */
    lru_move_to_back(main_mem->frame_tbl,main_mem->frame_tbl->table+(physical_address>>PT_SHIFT));
    /* Working set bit set automatically in tlb for each get */
}
/* 
Write a block to main memory:
Error checking not required as it is gauranteed the page is in memory at this point 
*/
void write_main_memory(main_memory* main_mem, trans_look_buff* tlb, uint32_t physical_address){
    /* Move entry to back of LRU Queue (Make MRU frame) */
    lru_move_to_back(main_mem->frame_tbl,main_mem->frame_tbl->table+(physical_address>>PT_SHIFT));
    /* Working set bit set automatically in tlb for each get */
    /* Bit set in TLB, after replacement, this is written to main memory */
    set_dirty_bit_tlb(tlb, physical_address>>PT_SHIFT);
}

/*
Periodically called via interrupts to detect thrashing
*/
void working_set_interrupt_handler(int sig){
    int count_per_process[gtasks->list->node_count];
    int total_count=0;                                  /* Total Frame Count */
    if (isEmpty(gtasks->list))
        return;
    q_node* curr = gtasks->list->front;
    int i=0;
    /* Iterate through all tasks */
    while(curr != NULL) {
        task_struct* task = ((task_struct*)(curr->data_ptr));
        count_per_process[i]=0;
        
        /* Don't bother if already swapped out */
        if(task->status != SWAPPED_OUT){
            /* Visit all pages */
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
                            /* Get the working set bits */
                            uint32_t working_set_bits = (*pt_ent)&WORKING_SET_MASK;
                            if(working_set_bits){
                                count_per_process[i]++;
                            }
                            /* Right shift */
                            working_set_bits = (working_set_bits>>(WORKING_SET_SHIFT+1))<<WORKING_SET_SHIFT;
                            (*pt_ent) = reset_bit_pgtbl_entry((*pt_ent),WORKING_SET_MASK);
                            (*pt_ent) |= working_set_bits; 
                        }
                    }
                }
            }
            total_count+=count_per_process[i];
            if(count_per_process[i]>task->stat.max_working_set){
                task->stat.max_working_set = count_per_process[i];
            }
        }
        i++;
        curr = curr->next;
    }
    /* Saving for statistics */
    working_set_counts[working_set_counts_index] = total_count;
    working_set_counts_index++;
    if(total_count>=UPPER_LIMIT_THRASHING){
        frequency_thrashing++;
        /* Block new processes or swapped out processes */
        gm_subsys->main_mem->thrashing = 1; 
        
        /* Search for a victim process */
        q_node* swap_node = gtasks->list->front;
        while(swap_node != NULL) {
            if(((task_struct*)(swap_node->data_ptr))->status == READY || ((task_struct*)(swap_node->data_ptr))->status == WAITING){
                break;
            }
            swap_node = swap_node->next;
        }
        /* suspend the task */
        if(swap_node!=NULL){
            unload_task(gm_subsys->main_mem, swap_node->data_ptr, 1);
        }
    }
    else if(total_count<LOWER_LIMIT_THRASHING){
        /* Allow new processes */
        gm_subsys->main_mem->thrashing = 0;
    }
    /* Reset Timer for Working set */
    signal(SIGVTALRM,working_set_interrupt_handler);
    struct timeval interval;
    struct timeval zero;
    struct itimerval period;
    interval.tv_sec=0;
    interval.tv_usec=TIMER_INTERVAL;
    zero.tv_sec=0;
    zero.tv_usec=0;
    period.it_interval=zero;
    period.it_value=interval;
    setitimer(ITIMER_VIRTUAL,&period,NULL);
}