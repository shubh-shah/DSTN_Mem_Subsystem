#include "main_memory.h"

// Do we differentiate betwen first and subsequent swaps?-Disk map or direct mapping?
// Implemented Direct Mapping assuming contiguous allocation in secondary memory

void swap_in(main_memory* main_mem, task_struct* task, uint32_t frame_no, uint32_t linear_address){
    //Lock frame
    //Pread
    for(int i=0;i<PG_SIZE;i++){
        main_mem[(frame_no<<PT_SHIFT)+i]=sec_mem[task->sec_mem_br+((linear_address>>PT_SHIFT)<<PT_SHIFT)+i];
    }
    //Unlock Frame
}

extern void swap_out(main_memory* main_mem, task_struct* task, uint32_t frame_no, uint32_t linear_address){
    if(dirty){
        for(int i=0;i<PG_SIZE;i++){
            sec_mem[task->sec_mem_br+((linear_address>>PT_SHIFT)<<PT_SHIFT)+i]=main_mem[(frame_no<<PT_SHIFT)+i];
        }
    }
    
}