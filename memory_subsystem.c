#include "memory_subsystem.h"

/*
    Return Value:Success(1) or Failure(0)
    If success, value of loaded byte stored in/from register
    Linear address = 32 bits (4bytes-int)
*/
bool load_store_byte(memory_subsystem* mem, task_struct* task, unsigned int linear_address, bool load){
    int offset,page_no,frame_no;
restart:
    //Split Linear Address
    offset = linear_address&(0x1FF);
    page_no = linear_address>>9;
    //Query TLB
    frame_no = get_frame_no_tlb(mem,task,page_no);
    //TLB Miss
    if(!is_valid_frame(frame_no)){
        //Page fault would occur here:
        if(!get_frame_no_mem(mem,task,page_no)){
            //This function would fork for sec mem access
            do_page_fault(mem,task,linear_address);
            //Yeild
            return 0;
        }
        goto restart;
    }
    int physical_address = (frame_no<<9)+offset;
    //Return value would be success or failure
    if(load_store_l1cache(mem,physical_address)){
        return 1;
    }
    if(load_store_l2cache(mem,physical_address)){
        return 1;
    }
    //Error checking not required as it is gauranteed the page is in memory at this point
    if(load){
        mem->reg=mem->main_mem[physical_address];
    }
    else{
        mem->main_mem[physical_address]=mem->reg;
    }
    return 1;
}