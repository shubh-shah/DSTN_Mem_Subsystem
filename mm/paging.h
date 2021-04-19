#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include "../util/queue.h"
#define MM_SIZE (32*1024*1024)                      /* 32MB */
#define PG_SIZE 512                                 /* 512B */

/* Page Table Constants */
#define PG_TBL_ENTRY_SIZE 4
#define ENTRY_PER_PG (PG_SIZE/PG_TBL_ENTRY_SIZE)    /* 512/4 */
#define PGD_SHIFT 30                                /* Bits to shift to get offset in pgd */
#define PMD_SHIFT 23
#define PLD_SHIFT 16
#define PT_SHIFT 9
/* Masks to get different bits */
#define PAGE_MASK 0xFFFF                            /* Mask to get frame no from pg table entry */
#define VALID_MASK 0x10000
#define DIRTY_MASK 0x20000
#define WORKING_SET_MASK 0xFFC0000
#define WORKING_SET_BIT 0x8000000
#define GLOBAL_MASK 0x10000000
#define WORKING_SET_SHIFT 18

/* Frame Table Constants */
#define FRAME_TBL_ENTRY_SIZE 4
#define NUM_FRAMES (MM_SIZE/PG_SIZE)                /* 64K */

/*
    Memory Structure:
    Bits required for logical address: 32 (2-pgd,7-pmd,7-pld,7-pt,9-offset)
    Bits for offset: 9
    Bits to address a page:32-9=23
    Bits required for physical address: 25

    PAGE TABLE Entry(4Bytes) Bits: FrameNo:0-15, Valid-16, Dirty-17, Working Set: 18-27 ,Global-28
    PAGE DIR Entry(4Bytes) Bits: FrameNo-0-15, Valid-16

    Heirarchy is:
    Page global directory(pgd)->Page Middle directory(pmd)->Page Lower directory(pld)->Page Table(pt)->offset
*/

/* Page Table */
#define is_valid_frame_no(frame_no) (frame_no<NUM_FRAMES)
#define is_valid_entry(pte) (pte&VALID_MASK)

/* For Page table walk */
#define pgd_index(linear_address) ((linear_address >> PGD_SHIFT) & (ENTRY_PER_PG-1))
#define pgd_entry_from_offset(task, offset) (((uint32_t*)(task)->pgd)+offset)
#define pgd_entry(task, linear_address) (((uint32_t*)(task)->pgd)+pgd_index(linear_address))

#define pmd_index(linear_address) ((linear_address >> PMD_SHIFT) & (ENTRY_PER_PG-1))
#define pmd_entry_from_offset(mem_begin, dir_entry, offset) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+offset)
#define pmd_entry(mem_begin, dir_entry, linear_address) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+pmd_index(linear_address))

#define pld_index(linear_address) ((linear_address >> PLD_SHIFT) & (ENTRY_PER_PG-1))
#define pld_entry_from_offset(mem_begin, dir_entry, offset) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+offset)
#define pld_entry(mem_begin, dir_entry, linear_address) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+pld_index(linear_address))

#define pt_index(linear_address) ((linear_address >> PT_SHIFT) & (ENTRY_PER_PG-1))
#define pt_entry_from_offset(mem_begin, dir_entry, offset) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+offset)
#define pt_entry(mem_begin, dir_entry, linear_address) (((uint32_t*)(mem_begin+((dir_entry&PAGE_MASK)<<PT_SHIFT)))+pt_index(linear_address))

#define reset_bit_pgtbl_entry(entry,mask) (entry)&(~((uint32_t)(mask)))

#define page_table_entry_to_frame_table_entry_ptr(frame_table_begin, pt_entry) (frame_table_begin+((pt_entry)&PAGE_MASK))

/* Frame Table */
typedef struct{
    uint32_t* page_table_entry;
    int pid;    /* PID=-1 means global frame */
    bool valid;
} frame_table_entry;

typedef struct{
    frame_table_entry table[NUM_FRAMES];
    queue* lru;
} frame_table;

extern frame_table* init_frame_table();

extern void lru_move_to_back(frame_table* frame_tbl, frame_table_entry* frtbl_ent);

extern frame_table_entry* lru_remove_by_frame_tbl_entry(frame_table* frame_tbl, frame_table_entry* frtbl_ent);

extern frame_table_entry* lru_remove_by_pid(frame_table* frame_tbl, int pid);

#endif