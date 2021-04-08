#include "main_memory.h"
#include <pthread.h>

extern void init_main_memory(main_memory* main_mem){
    main_mem->free_frames = NUM_FRAMES;
    for(int i=0;i<NUM_FRAMES;i++){
        main_mem->frame_table[i].valid = 0;
    }
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

void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t* invalid_entry, uint32_t linear_address, bool is_pgtbl){
    if(is_valid_entry(*invalid_entry)){
        return;
    }
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
    pthread_create(&tid_listen,&attr,do_page_fault,(void*)args);
}

//Synchronistaion problems??? Lock a page table????
void __do_page_fault(void* args){ 
    uint32_t linear_address = ((struct pass_pg_fault*) args)->linear_address;
    uint32_t* entry = ((struct pass_pg_fault*) args)->invalid_entry;
    bool is_pgtbl = ((struct pass_pg_fault*) args)->is_pgtbl;
    main_memory* main_mem = ((struct pass_pg_fault*) args)->main_mem;
    task_struct* task = ((struct pass_pg_fault*) args)->task;
    free((struct pass_pg_fault*) args);
    if(is_valid_entry(*entry)){
        return;
    }
	if (linear_address < TASK_SIZE) {
        uint32_t frame_no = get_zeroed_page(main_mem, task);
        if(!is_valid_frame_no(frame_no)){
            ERRORR;
        }
        //Set the entry:
        *entry = frame_no|VALID_MASK;
        if(!is_pgtbl){
            swap_in();
        }
        return;
	}
}

uint32_t get_zeroed_page(main_memory* main_mem, task_struct* task){
    if(main_mem->free_frames == 0){
        replace_a_frame;
    }
    for(uint32_t i=0;i<NUM_FRAMES;i++){
        if(!main_mem->frame_table[i].valid){
            main_mem->frame_table[i].valid = 1;
            main_mem->frame_table[i].page_number = ;//What to do for pages of page tables? 
            main_mem->frame_table[i].pid = task->pid;
            main_mem->free_frames--;
            return i;
        }
    }
    return 0xFFFFFFFF;
}


// void init_main_memory(memory_subsystem mem){
//     bzero(mem.memory,MM_SIZE);
//     //Make all frames coressponding to frame table valid SHould we do this??
//     for(int i=0;i<NUM_FRAMES/PG_SIZE;i++){
//         mem.memory[FRAME_TBL_START+i*4]=1<<8;//Set valid
//     }
// }

// int find_free_frame(memory_subsystem mem){
//     for(int i=0;i<NUM_FRAMES;i++){
//         if((1<<8)^mem.memory[FRAME_TBL_START+4*i]){
//             return i;
//         }
//     }
//     return INVALID;
// }

// int get_frame_no_from_pg_table_entry(int pg_dir_offset,int ptbr,int ptlr){
//     // for(i=0;i<4)
//     if(pg_dir_0_entry<ptlr){ //or should it be pg_dir_offset?
//         load_byte_mem(mem.mem,ptbr+4*pg_dir_0_entry); //get 4 bytes;
//     }
//     //check permission and valid and stuff and raise signals of page fault
// }

// char load_byte_mem(memory_subsystem mem, int physical_address){

// }

// void store_byte_mem(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);