#include "main_memory.h"

void init_main_memory(main_memory mem){
    bzero(mem.memory,MM_SIZE);
    //Make all frames coressponding to frame table valid SHould we do this??
    for(int i=0;i<NUM_FRAMES/PG_SIZE;i++){
        mem.memory[FRAME_TBL_START+i*4]=1<<8;//Set valid
    }
}

int find_free_frame(main_memory mem){
    for(int i=0;i<NUM_FRAMES;i++){
        if((1<<8)^mem.memory[FRAME_TBL_START+4*i]){
            return i;
        }
    }
    return INVALID;
}


extern int get_frame_number_from_pg_number_main_memory(main_memory mem,int page_number){
    int pg_tbl_entry = (page_number)&((1<<7)-1);
    page_number=page_number>>7;
    int pg_dir_entry[3];
    int pg_dir_entry[2] = (page_number)&((1<<7)-1);
    page_number=page_number>>7;
    int pg_dir_entry[1] = (page_number)&((1<<7)-1);
    page_number=page_number>>7;
    int pg_dir_entry[0] = (page_number)&((1<<7)-1);
    for(int i=0;i<3;i++){
        ptbr=get_frame_number_from_pg_dir_entry(pg_dir_entry[i],ptbr,ptlr);
        ptlr=NO_OF_ENTRIES_PER_PAGE;
    }
    get_frame_number_from_pg_table_entry(pg_tbl_entry,ptbr,ptlr);
}

int get_frame_number_from_pg_dir_entry(int pg_dir_entry,int ptbr,int ptlr){
    // for(i=0;i<4)
    if(pg_dir_0_entry<ptlr){ //or should it be pg_dir_offset?
        load_byte_mem(mem.main_mem,ptbr+4*pg_dir_0_entry); //get 4 bytes;
    }
    //check permission and valid and stuff and raise signals of page fault
}

int get_frame_number_from_pg_table_entry(int pg_dir_entry,int ptbr,int ptlr){
    // for(i=0;i<4)
    if(pg_dir_0_entry<ptlr){ //or should it be pg_dir_offset?
        load_byte_mem(mem.main_mem,ptbr+4*pg_dir_0_entry); //get 4 bytes;
    }
    //check permission and valid and stuff and raise signals of page fault
}

// char load_byte_mem(main_memory mem, int physical_address){

// }

// void store_byte_mem(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);