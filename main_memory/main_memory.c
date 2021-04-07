#include "main_memory.h"

/*Page fault handling?*/
extern int get_frame_no_mem(memory_subsystem* mem, task_struct* task, int page_no){
    int pg_tbl_entry = (page_no)&((1<<7)-1);
    page_no=page_no>>7;
    int pg_dir_entry[3];
    int pg_dir_entry[2] = (page_no)&((1<<7)-1);
    page_no=page_no>>7;
    int pg_dir_entry[1] = (page_no)&((1<<7)-1);
    page_no=page_no>>7;
    int pg_dir_entry[0] = (page_no)&((1<<7)-1);
    int pbr = task->ptbr;
    int plr = task->ptlr;
    for(int i=0;i<3;i++){
        pbr = get_tbl_frame_no(mem,pg_dir_entry[i],pbr,plr);
        plr = NUM_PG_TBL_ENTRIES_PER_PG;
    }
    get_frame_no_from_pg_table_entry(mem,pg_tbl_entry,pbr,plr);
}

int get_tbl_frame_no(memory_subsystem* mem,int pg_dir_entry,int ptbr,int ptlr){
    // for(i=0;i<4)
    if(pg_dir_entry < ptlr){ //or should it be pg_dir_offset?
        uint32_t entry = *((uint32_t*)(mem->main_mem+ptbr+4*pg_dir_entry));
    }
    //check permission and valid and stuff and raise signals of page fault
    return ;
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

// int get_frame_no_from_pg_table_entry(int pg_dir_entry,int ptbr,int ptlr){
//     // for(i=0;i<4)
//     if(pg_dir_0_entry<ptlr){ //or should it be pg_dir_offset?
//         load_byte_mem(mem.mem,ptbr+4*pg_dir_0_entry); //get 4 bytes;
//     }
//     //check permission and valid and stuff and raise signals of page fault
// }

// char load_byte_mem(memory_subsystem mem, int physical_address){

// }

// void store_byte_mem(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);