#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mem_struct.h"
#include "task.h"
#include "memory_subsystem.h"

int main(){
    srand(time(0));
    FILE* traces[5];
    traces[0]=fopen("inputs/2019_20_IISEM_APSI.txt","r");
    traces[1]=fopen("inputs/2019_20_IISEM_CC1.txt","r");
    traces[2]=fopen("inputs/2019_20_IISEM_LI.txt","r");
    traces[3]=fopen("inputs/2019_20_IISEM_M88KSIM.txt","r");
    traces[4]=fopen("inputs/2019_20_IISEM_VORTEX.txt","r");

    int curr_process = 0;
    int done[5]={0};    //Replace with task->status
    task_struct task[5];

    memory_subsystem* mem=malloc(sizeof(memory_subsystem));
    init_memory_subsystem(mem);

    uint32_t logical_address;
    
    while(!(done[0] && done[1] && done[2] && done[3] && done[4])){
        int time = (rand()%100+150);
        while(time-- && !(done[curr_process] = (fscanf(traces[curr_process],"%x",logical_address)<=0))){
            // if(load_byte(mem, task+curr_process, logical_address)){
                //Page fault
                //Change process
            // }
        }
        curr_process=(curr_process+1)%5;
    }
}