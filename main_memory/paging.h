#include <stdint.h>
#include <stdbool.h>
#define MM_SIZE (32*1024*1024)          //32MB
#define PG_SIZE 512                  //512B
#define TASK_SIZE (4*1024*1024*1024)

//Page Table Constants
#define PG_TBL_ENTRY_SIZE 4
#define ENTRY_PER_PG (PG_SIZE/PG_TBL_ENTRY_SIZE)    //512/4
#define PGD_SHIFT 30    //Bits to shift to get offset in pgd
#define PMD_SHIFT 23
#define PLD_SHIFT 16
#define PT_SHIFT 9
#define PAGE_MASK 0xFFFF    //Mask to get frame no from pg table entry
#define VALID_MASK 0x10000
//Frame Table Constants
#define FRAME_TBL_ENTRY_SIZE 4
#define NUM_FRAMES (MM_SIZE/PG_SIZE)    //64K

/*
pgd
pmd
pld
pt
*/

/*
    Memory Structure:
    Starting 10 Bytes for process details : PTBR, PTLR
    Bits required for logical address: 32 (2,7,7,7,9)
    Bits for offset: 9
    Bits to address a page:32-9=23
    Bits required for physical address: 25

    Next 4*64KB for Frame Table:
        FRAME TABLE Entry(4Bytes): Valid-1,PgNo-23,PID-8
        Pages required: 4*64K/512 = 4*64*2=512
    PAGE TABLE Entry(4Bytes): FrameNo:0-15,Valid-16,Global-17,Dirty-18,LRUbits:??,Caching-1,Protection-?? (12 bits left)
    PAGE DIR Entry(4Bytes): FrameNo-0-15,Valid-16,LRUbits:??,Caching-1,Protection-??
    //Do we want multiple size pages?
    //Make page table frames unswappable? 
    //Free frame list??
*/

typedef struct{
    uint32_t page_number;
    int pid;
    bool valid;
} frame_table_entry;

#define is_valid_frame_no(frame_no) (frame_no<NUM_FRAMES)
#define is_valid_entry(pte) (pte&VALID_MASK)

#define pgd_index(linear_address) ((linear_address >> PGD_SHIFT) & (ENTRY_PER_PG-1))
#define pgd_entry(task, linear_address) (((uint32_t*)(task)->pgd)+pgd_index(linear_address))

#define pmd_index(linear_address) ((linear_address >> PMD_SHIFT) & (ENTRY_PER_PG-1))
#define pmd_entry(mem_begin, dir_entry, linear_address) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+pmd_index(linear_address))
//If doesn't work, change to: entry = *((uint32_t*)(main_mem->mem_arr+ptbr+PG_TBL_ENTRY_SIZE*pg_dir_offset));-Incorrect as frame no only, not address of frame

#define pld_index(linear_address) ((linear_address >> PLD_SHIFT) & (ENTRY_PER_PG-1))
#define pmd_entry(mem_begin, dir_entry, linear_address) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+pld_index(linear_address))

#define pt_index(linear_address) ((linear_address >> PT_SHIFT) & (ENTRY_PER_PG-1))
#define pt_entry(mem_begin, dir_entry, linear_address) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+pt_index(linear_address))