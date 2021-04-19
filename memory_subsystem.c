#include "memory_subsystem.h"
#include <malloc.h>

memory_subsystem* init_memory_subsystem(){
    memory_subsystem* mem = malloc(sizeof(memory_subsystem));
    mem->tlb = init_tlb();
    mem->l1cache = init_l1_cache();
    mem->l2cache = init_l2_cache();
    mem->main_mem = init_main_memory();
    return mem;
}

/*
    Input:
        Linear address = 32 bits (4bytes-int)
    Returns:
        Success(0): Value of byte placed in mem->reg
        Failure(1): Value of mem->reg set to error code
*/
bool load_store_byte(memory_subsystem* mem, task_struct* task, uint32_t linear_address, bool load){
    task->stat.references++;
    uint32_t offset,frame_no;
restart:
    /* Split Linear Address */
    offset = linear_address & ((uint32_t)0x1FF);
    frame_no = get_frame_no_tlb(mem->tlb,task,linear_address);
    /* TLB Miss Handling */
    if(!is_valid_frame_no(frame_no)){
        int error;
        /* Page fault/Invalid Ref Handling */
        if((error = do_page_table_walk(mem->main_mem,mem->tlb,task,linear_address))){
            /* The error code is forwarded */
            mem->reg = error;    
            return 1;
        }
        /* Restart instruction */
        goto restart;
    }
    uint32_t physical_address = (frame_no<<PT_SHIFT)+offset;

    if(load){
        task->stat.l1_read_access++;
        if(!read_l1_cache(mem->l1cache,physical_address)){
            task->stat.l1_read_miss++;
            /* Look Through for l1*/
            bool l2_miss = !read_l2_cache(mem->l2cache,physical_address);
            /* Look aside for l2 */
            read_main_memory(mem->main_mem,physical_address);
            if(l2_miss){
                task->stat.l2_miss++;
                update_l2_cache(mem->l2cache,mem->main_mem,physical_address);
            }
            update_l1_cache(mem->l1cache,physical_address);
            read_l1_cache(mem->l1cache,physical_address);
        }
    }
    else{
        task->stat.l1_write_access++;
        if(!write_l1_cache(mem->l1cache, physical_address)){
            task->stat.l1_write_miss++;
            /* Write allocate mechanism for write misses - Write miss same as read miss - get data to l1 */
            bool l2_miss = !read_l2_cache(mem->l2cache,physical_address);
            /* Look aside for l2 */
            read_main_memory(mem->main_mem,physical_address);
            if(l2_miss){
                task->stat.l2_miss++;
                update_l2_cache(mem->l2cache,mem->main_mem,physical_address);
            }
            update_l1_cache(mem->l1cache,physical_address);
            /* Write will succeed now */
            write_l1_cache(mem->l1cache, physical_address);
        }
        /* Write through for l1 cache - no checking because we know the data is already in l2 cache */
        write_l2_cache(mem->l2cache,physical_address);
    }
    return 0;
}