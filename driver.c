#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "task.h"
#include "memory_subsystem.h"
#include "global_variables.h"

task_list* gtasks;
memory_subsystem* gm_subsys;

int main(){
    srand(92);
    FILE* traces[5];
    traces[0]=fopen("inputs/2019_20_IISEM_APSI.txt","r");
    traces[1]=fopen("inputs/2019_20_IISEM_CC1.txt","r");
    traces[2]=fopen("inputs/2019_20_IISEM_LI.txt","r");
    traces[3]=fopen("inputs/2019_20_IISEM_M88KSIM.txt","r");
    traces[4]=fopen("inputs/2019_20_IISEM_VORTEX.txt","r");

    gm_subsys = init_memory_subsystem();
    printf("Memory System Started!\n");
    gtasks = init_task_list();
    int pid[5];
    for(int i=0;i<5;i++){
        while((pid[i]=init_task())!=-1){
            //Means THrashing is on!
            sleep(1);
        } 
    }
    printf("Tasks Initialised!\n");
    int curr_task=0;
    uint32_t logical_address;
    while(!isEmpty(gtasks->list)){
        if(run_task(pid[curr_task])){
            int allowed_refrences = (rand()%100+150);
            printf("Task %d started.\n",pid[curr_task]);
            int error;
            while(allowed_refrences-- && (find_task(pid[curr_task]))->status==RUNNING){
                if(fscanf(traces[curr_task],"%x",&logical_address)<=0){
                    destroy_task(pid[curr_task]);
                    break;
                }
                if( (logical_address&0x7F000000) == 0x7F000000){ //code_segment
                    if(error = load_byte(gm_subsys, (find_task(pid[curr_task])), logical_address))
                        break;    /* Page fault */               
                }
                else{
                    if(error = load_store_byte(gm_subsys, (find_task(pid[curr_task])), logical_address,rand()%2))   /* rand()%2 decides the op:load or store */
                        break;    /* Page fault */  
                }
            }
            preempt_task(pid[curr_task]);
        }
        curr_task=(curr_task+1)%5;
    }
}