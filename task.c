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
        task_struct* task = malloc(sizeof(task_struct));
        task->pid = gtasks->next_pid;
        task->frames_used = 0;
        task->ptlr = 4;
        task->status = READY;
        gtasks->next_pid++;
        uint32_t frame_no = get_zeroed_page(gm_subsys->main_mem,task,task->pgd,1);
        task->pgd = ((uint32_t*)(gm_subsys->main_mem->mem_arr+(frame_no<<PT_SHIFT))); 
        push(gtasks->list, task);
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
    return curr;
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
    unload_task(gm_subsys->main_mem,curr,0);    //Imported from main_memory.h
    free(curr->data_ptr);
    free(curr);
    return 1;
}