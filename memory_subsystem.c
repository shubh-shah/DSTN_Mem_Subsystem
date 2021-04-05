#include "memory_subsystem.h"

//Linear address = 32 bits (4bytes-int)

char load_byte(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address){
    int physical_address = translate(mem,linear_address,page_table_base_register,page_table_length_register);

}

void store_byte(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);

int translate(memory_subsystem mem,int linear_address, int ptbr, int ptlr){
    //Split Linear Address:
    int offset = linear_address&((1<<9)-1);
    int page_no = linear_address>>9;
    //DO miss
    int frame_no=get_frame_number_from_pg_number_tlb(mem.tlb,page_number);
    if(tlb does not have)
        frame_no=get_frame_number_from_pg_number_main_memory(mem.main_mem,page_number);
    //Can we put page table entries in tlb?
    return (frame_no<<9)+offset;
}