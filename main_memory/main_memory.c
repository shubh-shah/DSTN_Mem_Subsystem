#include "main_memory.h"

/*
    Input:
        Page No. (23 bits)
    Returns:
        Successful: 0
        Invalid Refrence: UINT32_MAX-1 (FFFFFFFE)
        Page Fault: UINT32_MAX (FFFFFFFF)
*/
int do_page_table_walk(main_memory* main_mem, trans_look_buff* tlb, task_struct* task, uint32_t page_no){
    uint8_t pg_tbl_offset = (page_no)&(0x7F);
    page_no=page_no>>7;
    uint8_t pg_dir_offset[3];
    pg_dir_offset[2] = (page_no)&(0x7F);
    page_no=page_no>>7;
    pg_dir_offset[1] = (page_no)&(0x7F);
    page_no=page_no>>7;
    pg_dir_offset[0] = (page_no)&(0x7F);

    uint32_t pbr = task->ptbr;
    uint32_t plr = task->ptlr;
    for(int i=0;i<3;i++){
        if((pbr = traverse_pgdir(main_mem,pg_dir_offset[i],pbr,plr)) >= NUM_FRAMES)
            return pbr;
        plr = NUM_PG_TBL_ENTRIES_PER_PG;    //Change??
    }

    if(pg_tbl_offset >= plr)           //Doubt??
        return INVALID_REF;            //Invalid Reference
    uint32_t page_table_entry = *((uint32_t*)(main_mem->mem_arr+pbr+PG_TBL_ENTRY_SIZE*pg_tbl_offset));
    if(page_table_entry^(0x10000))
        return PAGE_FAULT;              //Page Fault
    insert_tlb_entry(tlb, task, page_no, page_table_entry);
    return 0;
}

/*
    Input:
        Index in the page table/Dir (7 bit)
    Returns:
        Successful: Frame Number of the required page (16 bits)
        Invalid Refrence: UINT32_MAX-1 (FFFFFFFE)
        Page Fault: UINT32_MAX (FFFFFFFF)
*/
uint32_t traverse_pgdir(main_memory* main_mem,uint8_t pg_dir_offset,uint32_t ptbr,uint32_t ptlr){
    if(pg_dir_offset >= ptlr)           //Doubt??
        return INVALID_REF;            //Invalid Reference
    
    uint32_t entry = *((uint32_t*)(main_mem->mem_arr+ptbr+PG_TBL_ENTRY_SIZE*pg_dir_offset));
    if(entry^(0x10000))
        return PAGE_FAULT;              //Page Fault
    return entry&((uint32_t)0xFFFF);
}

/*
    From Linux 1.0 Kernel: do_no_page
    This function would fork for sec mem access
*/
void do_page_fault(main_memory* main_mem, task_struct* task, uint32_t linear_address)
{
	unsigned long tmp;
	unsigned long page;
	struct vm_area_struct * mpnt;

	page = get_empty_pgtable(tsk,address);
	if (!page)
		return;
	page &= PAGE_MASK;
	page += PAGE_PTR(address);
	tmp = *(unsigned long *) page;
	if (tmp & PAGE_PRESENT)
		return;
	++tsk->rss;
	if (tmp) {
		++tsk->maj_flt;
		swap_in((unsigned long *) page);
		return;
	}
	address &= 0xfffff000;
	tmp = 0;
	for (mpnt = tsk->mmap; mpnt != NULL; mpnt = mpnt->vm_next) {
		if (address < mpnt->vm_start)
			break;
		if (address >= mpnt->vm_end) {
			tmp = mpnt->vm_end;
			continue;
		}
		if (!mpnt->vm_ops || !mpnt->vm_ops->nopage) {
			++tsk->min_flt;
			get_empty_page(tsk,address);
			return;
		}
		mpnt->vm_ops->nopage(error_code, mpnt, address);
		return;
	}
	if (tsk != current)
		goto ok_no_page;
	if (address >= tsk->end_data && address < tsk->brk)
		goto ok_no_page;
	if (mpnt && mpnt == tsk->stk_vma &&
	    address - tmp > mpnt->vm_start - address &&
	    tsk->rlim[RLIMIT_STACK].rlim_cur > mpnt->vm_end - address) {
		mpnt->vm_start = address;
		goto ok_no_page;
	}
	tsk->tss.cr2 = address;
	current->tss.error_code = error_code;
	current->tss.trap_no = 14;
	send_sig(SIGSEGV,tsk,1);
	if (error_code & 4)	/* user level access? */
		return;
ok_no_page:
	++tsk->min_flt;
	get_empty_page(tsk,address);
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

// int get_frame_no_from_pg_table_entry(int pg_dir_offset,int ptbr,int ptlr){
//     // for(i=0;i<4)
//     if(pg_dir_0_entry<ptlr){ //or should it be pg_dir_offset?
//         load_byte_mem(mem.mem,ptbr+4*pg_dir_0_entry); //get 4 bytes;
//     }
//     //check permission and valid and stuff and raise signals of page fault
// }

// char load_byte_mem(memory_subsystem mem, int physical_address){

// }

// void store_byte_mem(memory_subsystem mem, int page_table_base_register, int page_table_length_register, int linear_address, char data);