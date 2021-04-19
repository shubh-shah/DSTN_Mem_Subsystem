#include "task.h"
#include "global_variables.h"
#include "mm/main_memory.h"

task_list* init_task_list(){
    task_list* tl = malloc(sizeof(task_list));
    tl->list = createQueue(1024);
    tl->next_pid = 0;
    return tl;
}

int init_task(){
    if(!(gm_subsys->main_mem->thrashing)){
        /* Initialize a new task */
        task_struct* task = malloc(sizeof(task_struct));
        task->pid = gtasks->next_pid;
        task->frames_used = 0;
        task->ptlr = 4;
        task->status = READY;
        gtasks->next_pid++;
        /* Allocate frame for pgd */
        task->pgd = malloc(sizeof(uint32_t));
        uint32_t frame_no = get_zeroed_page(gm_subsys->main_mem,task,task->pgd,1);
        task->pgd = ((uint32_t*)(gm_subsys->main_mem->mem_arr+(frame_no<<PT_SHIFT))); 
        push(gtasks->list, task);
        /* Pre page first 2 blocks of memory - Page no 0,1 - Can do by calling page fault for linear address = 0,512 */
        uint32_t linear_address[2] = {0,512};
        uint32_t* pgd_ent = pgd_entry(task, linear_address[0]);
        do_page_fault(gm_subsys->main_mem, task, pgd_ent, linear_address[0], 1);

        uint32_t* pmd_ent = pmd_entry(gm_subsys->main_mem->mem_arr, *pgd_ent, linear_address[0]);
        do_page_fault(gm_subsys->main_mem, task, pmd_ent, linear_address[0], 1);

        uint32_t* pld_ent = pld_entry(gm_subsys->main_mem->mem_arr, *pmd_ent, linear_address[0]);
        do_page_fault(gm_subsys->main_mem, task, pld_ent, linear_address[0], 1);
        for(int i=0;i<2;i++){
            uint32_t* pt_ent = pt_entry(gm_subsys->main_mem->mem_arr, *pld_ent, linear_address[i]);
            do_page_fault(gm_subsys->main_mem, task, pt_ent, linear_address[i], 0);
        }
        return task->pid;
    }
    else{
        return -1;
    }
}

task_struct* find_task(int pid){
    if (isEmpty(gtasks->list))
        return NULL;
    q_node* curr = gtasks->list->front;
    while(((task_struct*)(curr->data_ptr))->pid != pid) {
        if(curr->next == NULL)
            return NULL;
        curr = curr->next;
    }
    return (task_struct*)curr->data_ptr;
}

bool run_task(int pid){
    task_struct* task = find_task(pid);
    if(task==NULL){
        return 0;
    }
    if(task->status == READY){
        task->status = RUNNING;
    }
    else if(task->status == SWAPPED_OUT && !(gm_subsys->main_mem->thrashing)){
        task->status = RUNNING;
    }
    else{
        return 0;
    }
    return 1;
}

bool preempt_task(int pid){
    task_struct* task = find_task(pid);
    if(task==NULL){
        return 0;
    }
    if(task->status == RUNNING){
        task->status = READY;
    }
    return 1;
}

bool destroy_task(int pid){
    if (isEmpty(gtasks->list))
        return 0;
    q_node* curr = gtasks->list->front;
    q_node* prev = 0;
    while(((task_struct*)(curr->data_ptr))->pid != pid) {
        if(curr->next == NULL)
            return 0;
        prev = curr;
        curr = curr->next;
    }
    unload_task(gm_subsys->main_mem,(task_struct*)curr->data_ptr,0);    /* Imported from main_memory.h */
    if(curr == gtasks->list->front){
        gtasks->list->front = gtasks->list->front->next;
    }
    else{
        prev->next = curr->next;
    }
    if(curr==gtasks->list->rear){
        gtasks->list->rear = prev;
    }
    gtasks->list->node_count--;
    free(curr->data_ptr);
    free(curr);
    return 1;
}