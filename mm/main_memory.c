#include "main_memory.h"
#include <pthread.h>
#include <malloc.h>
#include <sys/time.h>
#include <signal.h>

extern main_memory* init_main_memory(){
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
    Input:
        Page No. (23 bits)
    Returns:
        Successful 0
        Invalid Refrence
        Page Fault
*/
uint32_t do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t linear_address){
    //RD WR PERMISSION CHECK?
    //PTLR??
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
    /* Move entry to back of LRU Queue (Make MRU) */
    lru_move_to_back(main_mem->frame_tbl,pt_ent);
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
    *entry = frame_no;
    if(!is_pgtbl){
        if(swap_in(main_mem, task, entry)){     /* Page not in disk, Means dirty */
            *entry |= DIRTY_MASK;
        }
        else{
            *entry = reset_bit_pgtbl_entry((*entry),DIRTY_MASK);
        }
    }
    /* Update Page Table Entry */
    *entry |= VALID_MASK;
    task->status = READY;
    return NULL;
}
void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl){
    printf("Page Fault for: %x",linear_address);
    if(is_valid_entry(*invalid_entry)){
        return;
    }
    task->status = WAITING;
    pthread_t tid_listen;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    struct pass_pg_fault* args = malloc(sizeof(struct pass_pg_fault));
    args->invalid_entry = invalid_entry;
    args->linear_address = linear_address;
    args->is_pgtbl = is_pgtbl;
    args->main_mem = main_mem;
    args->task = task;
    // pthread_create(&tid_listen,&attr,_do_page_fault,(void*)args);
    _do_page_fault((void*)args);
}

uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl){
    frame_table_entry* lru_entry;
    if(main_mem->nr_free_frames == 0){     
        if (task->frames_used < MAX_FRAMES_PER_TASK) {
            lru_entry = (frame_table_entry*)(pop(main_mem->frame_tbl->lru)->data_ptr);
        }
        else{
            lru_entry = lru_remove_by_pid(main_mem->frame_tbl, task->pid);
        }
        swap_out(main_mem, lru_entry);
        *(lru_entry->page_table_entry) = reset_bit_pgtbl_entry((*(lru_entry->page_table_entry)),VALID_MASK);
        lru_entry->valid = 0;
        main_mem->nr_free_frames++;
        printf("Replacing Frame %d",lru_entry-main_mem->frame_tbl->table);
    }else{
        for(int i=0;i<NUM_FRAMES;i++){
            if(!main_mem->frame_tbl->table[i].valid){
                lru_entry = (main_mem->frame_tbl->table)+i;
                break;
            }
        }
    }
    //Lock frame -stop replacement
    //Unlock Frame
    //Lock process -stop recurrent page faults
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
    printf(" Frame Provided: %d\n",frame_no);
    return frame_no;
}

//Not called anywhere
uint32_t get_global_zeroed_page(main_memory* main_mem, task_struct* task, uint32_t* pgtbl_entry, bool is_pgtbl){
    uint32_t frame_no = get_zeroed_page(main_mem, task, pgtbl_entry, is_pgtbl);
    lru_remove_by_pgtbl_entry(main_mem->frame_tbl, pgtbl_entry);    /* Remove from lru queue to make it non replaceable */
    main_mem->frame_tbl->table[frame_no].pid = -1;
    *pgtbl_entry|=GLOBAL_MASK;
    return frame_no;
}

void working_set_interrupt_handler(int sig){
    //LOcking required here as well
    // time_t mytime = time(NULL);
    // char * time_str = ctime(&mytime);
    // time_str[strlen(time_str)-1] = '\0';
    // printf("Thrashing Routine Started: Time:%s\n",time_str);fflush(stdout);
    int no_of_tasks = gtasks->list->node_count;
    int count_per_process[no_of_tasks];
    int total_count=0;
    if (isEmpty(gtasks->list))
        return;
    q_node* curr = gtasks->list->front;
    int i=0;
    while(curr != NULL) {
        task_struct* task = ((task_struct*)(curr->data_ptr));
        count_per_process[i]=0;
        if(task->status != SWAPPED_OUT){
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
                uint32_t working_set_bits = (*pt_ent)&WORKING_SET_MASK;
                if(working_set_bits){
                    count_per_process[i]++;
                }
                working_set_bits = (working_set_bits>>(WORKING_SET_SHIFT+1))<<WORKING_SET_SHIFT;
                (*pt_ent) = reset_bit_pgtbl_entry((*pt_ent),WORKING_SET_MASK);
                (*pt_ent) |= working_set_bits; 
            }
            total_count+=count_per_process[i];
        }
        i++;
        curr = curr->next;
    }
    if(total_count>=UPPER_LIMIT_THRASHING){
        printf("Thrashing Detected %d! Slow down\n",total_count);
        gm_subsys->main_mem->thrashing = 1; //Block new processes
        
        q_node* swap_node = gtasks->list->front;
        //while == Nullswap_node = gtasks->list->front;??
        while(swap_node != NULL) {
            if(((task_struct*)(swap_node->data_ptr))->status == READY){
                break;
            }
            swap_node = swap_node->next;
        }
        unload_task(gm_subsys->main_mem, swap_node->data_ptr, 1);
        // need frame blocking here too if pthread
    }
    else if(total_count<LOWER_LIMIT_THRASHING){
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

//Should we write pages to memory when free?
//Keep a start searching pointer for get_zeroed_page?