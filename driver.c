#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "task.h"
#include "memory_subsystem.h"
#include "global_variables.h"

task_list* gtasks;
memory_subsystem* gm_subsys;
int working_set_counts[100];
int working_set_counts_index=0;

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
        while((pid[i]=init_task())==-1){
            /* Means thrashing */
            sleep(1);
        } 
    }
    printf("Tasks Initialised!\n");
    int curr_task=0;
    uint32_t logical_address;
    int64_t previous_address[5]={-1,-1,-1,-1,-1}; /*Storing previous address in case of page fault */
    while(!isEmpty(gtasks->list)){
        if(run_task(pid[curr_task])){
            int allowed_refrences = (rand()%100+150);
            int error;
            while(allowed_refrences-- && find_task(pid[curr_task])!=NULL){
                if((find_task(pid[curr_task]))->status!=RUNNING){
                    break;
                }
                if(previous_address[curr_task] == -1){
                    if(fscanf(traces[curr_task],"%x",&logical_address)<=0){
                        printf("Task %d Finished.\n",pid[curr_task]);fflush(stdout);
                        task_struct* task = find_task(pid[curr_task]);
                        printf("Statistics:\nReferences: %Ld\nTLB Misses: %Ld\n",task->stat.references,task->stat.tlb_miss);
                        printf("L1 Miss: %Ld\nL2 Miss: %Ld\n",task->stat.l1_miss,task->stat.l2_miss);
                        printf("Page Faults for page tables: %Ld\nPage faults for pages: %Ld\n",task->stat.page_fault,task->stat.page_fault_pt);
                        printf("Page replacements: %Ld\n",task->stat.page_replacements);
                        printf("Maximum working set size: %Ld\n",task->stat.max_working_set);
                        destroy_task(pid[curr_task]);
                        break;
                    }
                }
                else{
                    logical_address = previous_address[curr_task];
                }
                previous_address[curr_task] = -1;
                if( (logical_address&0x7F000000) == 0x7F000000){
                    /* code_segment - only load*/
                    if(error = load_byte(gm_subsys, (find_task(pid[curr_task])), logical_address)){
                        previous_address[curr_task] = logical_address;
                        break;    /* Page fault */     
                    }          
                }
                else{
                    /* rand()%2 decides the op:load or store */
                    if(error = load_store_byte(gm_subsys, (find_task(pid[curr_task])), logical_address,rand()%2)){
                        previous_address[curr_task] = logical_address;
                        break;    /* Page fault */  
                    }
                }
            }
            preempt_task(pid[curr_task]);
        }
        curr_task=(curr_task+1)%5;
    }
    printf("Overall Statistics\nWorking set sizes:\n");
    for(int i=0;i<working_set_counts_index;i++){
        printf("%d ",working_set_counts[i]);
    }
    printf("\n");
}