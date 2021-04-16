#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "task.h"
#include "memory_subsystem.h"

task_struct tasks[1024];
int no_of_tasks=0;

int is_unfinished(){
    for(int i=0;i<no_of_tasks;i++){
        if(tasks[i].status!=FINISHED)
            return 1;
    }
    return 0;
}

int main(){
    srand(92);
    FILE* traces[5];
    traces[0]=fopen("inputs/2019_20_IISEM_APSI.txt","r");
    traces[1]=fopen("inputs/2019_20_IISEM_CC1.txt","r");
    traces[2]=fopen("inputs/2019_20_IISEM_LI.txt","r");
    traces[3]=fopen("inputs/2019_20_IISEM_M88KSIM.txt","r");
    traces[4]=fopen("inputs/2019_20_IISEM_VORTEX.txt","r");

    memory_subsystem* mem = init_memory_subsystem();
    for(int i=0;i<5;i++){
        init_task(i,mem->main_mem);
        printf("%d %d\n",tasks[i].status,no_of_tasks);
    }
    int curr_task=0;
    uint32_t logical_address;
    while(is_unfinished()){
        if(tasks[curr_task].status == READY){
            int allowed_refrences = (rand()%100+150);
            if(fscanf(traces[curr_task],"%x",&logical_address)<=0){
                tasks[curr_task].status = FINISHED;
            }
            tasks[curr_task].status = RUNNING;
            while(allowed_refrences-- && tasks[curr_task].status==RUNNING){
                if(0){ //code_segment
                    if(load_byte(mem, tasks+curr_task, logical_address))
                        break;    /* Page fault */               
                }
                else{
                    if(load_store_byte(mem, tasks+curr_task, logical_address,rand()%2))   /* rand()%2 decides the op:load or store */
                        break;    /* Page fault */  
                }
            }
        }
        curr_task=(curr_task+1)%5;
    }
}