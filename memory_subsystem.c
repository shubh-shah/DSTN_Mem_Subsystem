#include "memory_subsystem.h"
#include <malloc.h>

extern memory_subsystem* init_memory_subsystem(){
    memory_subsystem* mem = malloc(sizeof(memory_subsystem));
    mem->tlb = init_tlb();
    // l1_cache l1cache;
    // l2_cache l2cache;
    mem->main_mem = init_main_memory();
    // init tasks
    return mem;
}

void init_task(int task_index, main_memory* main_mem){
    tasks[task_index].main_mem_base = main_mem->mem_arr;
    tasks[task_index].pid = no_of_tasks;
    tasks[task_index].frames_used = 0;
    tasks[task_index].ptlr = 2;
    tasks[task_index].main_mem_base = main_mem->mem_arr;
    tasks[task_index].status = READY;
    no_of_tasks++;
    uint32_t frame_no = get_zeroed_page(main_mem,tasks+task_index,tasks[task_index].pgd,1);
    uint32_t* pgd = ((uint32_t*)(main_mem->mem_arr+(frame_no<<PT_SHIFT)));
}

/*
    Input:
        Linear address = 32 bits (4bytes-int)
    Returns:
        Success(0): Value of byte placed in mem->reg
        Failure(1): Value of mem->reg set to error code
*/
bool load_store_byte(memory_subsystem* mem, task_struct* task, uint32_t linear_address, bool load){
    uint32_t offset,frame_no;
restart:
    /* Split Linear Address */
    offset = linear_address & ((uint32_t)0x1FF);
    frame_no = get_frame_no_tlb(mem->tlb,task,linear_address);
    /* TLB Miss Handling */
    if(!is_valid_frame_no(frame_no)){         
        int error;
        /* Page fault/Invalid Ref Handling */
        if(error = do_page_table_walk(mem->main_mem,mem->tlb,task,linear_address)){
            /* The error code is forwarded */
            mem->reg = error;    
            return 1;
        }
        /* Restart instruction */
        goto restart;
    }
    uint32_t physical_address = (frame_no<<PT_SHIFT)+offset;
    //Return value would be success or failure
    // if(!load_store_l1cache(mem->l1_cache,physical_address)){
        // return 0;
    // }
    // if(!load_store_l2cache(mem->l2_cache,physical_address)){
        // return 0;
    // }
    /* Error checking not required as it is gauranteed the page is in memory at this point */
    // In replacement for tlb set working bit in mm
    // set_working_set_bit_tlb(frame_no);
    if(load){
        //Load to l1&l2
        mem->reg=mem->main_mem->mem_arr[physical_address];
    }
    else{
        //Load to l1&l2
        // In replacement for tlb set dirty bit in mm
        // set_dirty_bit_tlb(frame_no);
        mem->main_mem->mem_arr[physical_address]=mem->reg;
    }
    return 0;
}