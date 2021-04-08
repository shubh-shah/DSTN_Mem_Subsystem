#include "memory_subsystem.h"
#include <malloc.h>

extern void init_memory_subsystem(memory_subsystem* mem){
    mem->tlb = malloc(sizeof(trans_look_buff));
    init_tlb(mem->tlb);
    // l1_cache l1cache;
    // l2_cache l2cache;
    mem->main_mem = malloc(sizeof(main_memory));
    init_main_memory(mem->main_mem);
    // sec_memory sec_mem;
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
    //Split Linear Address
    offset = linear_address & ((uint32_t)0x1FF);
    frame_no = get_frame_no_tlb(mem->tlb,task,linear_address);
    if(!is_valid_frame_no(frame_no)){         //TLB Miss
        int error;
        if(error = do_page_table_walk(mem->main_mem,mem->tlb,task,linear_address)){     //Page fault/Invalid Ref
            mem->reg = error;    //The error code is forwarded
            return 1;
        }
        //Restart instruction
        goto restart;
    }
    uint32_t physical_address = (frame_no<<PT_SHIFT)+offset;
    //Return value would be success or failure
    if(!load_store_l1cache(mem,physical_address)){
        return 0;
    }
    if(!load_store_l2cache(mem,physical_address)){
        return 0;
    }
    //Error checking not required as it is gauranteed the page is in memory at this point
    if(load){
        mem->reg=mem->main_mem->mem_arr[physical_address];
    }
    else{
        mem->main_mem->mem_arr[physical_address]=mem->reg;
    }
    return 0;
}