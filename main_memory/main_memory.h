#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#define MM_SIZE (32*1024*1024)          //32MB
#define PG_SIZE (512)                   //512B
#define NUM_FRAMES (MM_SIZE/PG_SIZE)    //64K

#define PG_TBL_ENTRY_SIZE 4
#define NUM_PG_TBL_ENTRIES_PER_PG (PG_SIZE/PG_TBL_ENTRY_SIZE)

#define FRAME_TBL_START 0
#define FRAME_TBL_END (NUM_FRAMES*4+FRAME_TABLE_START)

/*
    //Starting 10 Bytes for process details : PTBR, PTLR
    //Bits required for logical address: 32 (2,7,7,7,9)
    //Bits for offset: 9
    //Bits to address a page:32-9=23
    //Bits required for physical address: 25

    Next 4*64KB for Frame Table:
        FRAME TABLE Entry(4Bytes): Valid-1,PgNo-23,PID-8
        Pages required: 4*64K/512 = 4*64*2=512
    Free frame list??
    PAGE TABLE Entry(4Bytes): FrameNo-16,Global-1,Dirty-1,LRUbits:??,Caching-1,Protection-??,Valid-1 (12 bits left)
    PAGE DIR Entry(4Bytes): FrameNo-16,LRUbits:??,Caching-1,Protection-??,Valid-1
    //DO we want multiple size pages?
*/

typedef struct{
    char memory[MM_SIZE];
} main_memory;

/*
    Create and init Frame Tables

*/

extern void init_main_memory(main_memory mem);

extern int find_free_frame();

extern int get_frame_number_from_pg_number_main_memory(main_memory mem,int page_number);

// extern char load_byte_mem(main_memory mem, int physical_address);

// extern void store_byte_mem(main_memory mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);

// extern char* retreive_block(int start_add,int length);

// extern void store_block(int start_add,int length, char* data);

#endif