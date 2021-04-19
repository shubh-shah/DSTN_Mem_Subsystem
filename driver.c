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
/* For displaying statistics */
int working_set_counts[100];
int working_set_counts_index=0;
int frequency_thrashing=0;

int main(){
    srand(92);
    FILE* traces[5];
    traces[0]=fopen("inputs/2019_20_IISEM_APSI.txt","r");
    traces[1]=fopen("inputs/2019_20_IISEM_CC1.txt","r");
    traces[2]=fopen("inputs/2019_20_IISEM_LI.txt","r");
    traces[3]=fopen("inputs/2019_20_IISEM_M88KSIM.txt","r");
    traces[4]=fopen("inputs/2019_20_IISEM_VORTEX.txt","r");

    /* Initialize main memory and tasks */
    gm_subsys = init_memory_subsystem();
    printf("Memory System Started!\n");
    gtasks = init_task_list();
    int pid[5];    
    for(int i=0;i<5;i++){
        while((pid[i]=init_task(gtasks))==-1){
            /* Means thrashing */
            sleep(1);
        }
    }
    /* Overall Statistics */
    long long int references = 0;
    long long int tlb_miss = 0;
    long long int l2_miss = 0; 
    long long int l1_read_miss = 0; 
    long long int l1_write_miss = 0; 
    long long int l1_read_access = 0; 
    long long int l1_write_access = 0; 
    long long int page_fault = 0; 
    long long int page_fault_pt = 0; 
    long long int page_replacements = 0;
    long long int context_switches = 0;
    printf("Tasks Initialised!\n");
    int curr_task=0;
    uint32_t logical_address;
    int64_t previous_address[5]={-1,-1,-1,-1,-1}; /* For Storing previous address in case of page fault */
    while(!isEmpty(gtasks->list)){
        if(run_task(gtasks, pid[curr_task])){
            int allowed_refrences = (rand()%100+150);
            int error;
            while(allowed_refrences-- && find_task(gtasks, pid[curr_task])!=NULL){
                if((find_task(gtasks, pid[curr_task]))->status!=RUNNING){
                    break;
                }
                /* Scan new address only if page fault has not occured on previous try else call for the previous address*/
                if(previous_address[curr_task] == -1){
                    if(fscanf(traces[curr_task],"%x",&logical_address)<=0){
                        /* If no more input, print statistics and kill the process */
                        printf("--------------------------------------\nTask %d Finished. ",pid[curr_task]);fflush(stdout);
                        task_struct* task = find_task(gtasks, pid[curr_task]);
                        printf("Statistics:\n\tReferences: %Ld\n\t",task->stat.references);
                        printf("TLB Misses: %Ld (/%Ld)\n\t",task->stat.tlb_miss,task->stat.references);
                        printf("L1 Total Misses: %Ld (/%Ld)\n\t",task->stat.l1_read_miss+task->stat.l1_write_miss, task->stat.l1_read_access+task->stat.l1_write_access);
                        printf("\tRead Miss:%Ld (/%Ld)\n\t\tWrite Miss:%Ld (/%Ld)\n\t",task->stat.l1_read_miss,task->stat.l1_read_access,task->stat.l1_write_miss,task->stat.l1_write_access);
                        printf("L2 Miss: %Ld (/%Ld)\n\t",task->stat.l2_miss, task->stat.l1_read_miss+task->stat.l1_write_miss);
                        printf("Page faults for pages: %Ld (/%Ld)\n\tPage Faults for page tables: %Ld\n\t",task->stat.page_fault,task->stat.references,task->stat.page_fault_pt);
                        printf("Page replacements: %Ld\n\t",task->stat.page_replacements);
                        printf("Maximum working set size: %Ld\n\t",task->stat.max_working_set);
                        printf("No of times swapped out: %Ld\n",task->stat.swapped_out);
                        references+=task->stat.references;
                        tlb_miss+=task->stat.tlb_miss;
                        l2_miss+=task->stat.l2_miss;
                        l1_read_miss+=task->stat.l1_read_miss;
                        l1_write_miss+=task->stat.l1_write_miss;
                        l1_read_access+=task->stat.l1_read_access;
                        l1_write_access+=task->stat.l1_write_access;
                        page_fault+=task->stat.page_fault; 
                        page_fault_pt+=task->stat.page_fault_pt;
                        page_replacements+=task->stat.page_replacements;
                        destroy_task(gtasks, pid[curr_task]);
                        break;
                    }
                }
                else{
                    logical_address = previous_address[curr_task];
                }
                /* Reset previous address to -1, set to valid address only if page fault occurs */
                previous_address[curr_task] = -1;
                if( (logical_address&0x7F000000) == 0x7F000000){
                    /* code_segment - only load*/
                    if(error = load_byte(gm_subsys, (find_task(gtasks, pid[curr_task])), logical_address)){
                        previous_address[curr_task] = logical_address;
                        break;    /* Page fault */     
                    }          
                }
                else{
                    /* rand()%2 decides the op: load or store */
                    if(error = load_store_byte(gm_subsys, (find_task(gtasks, pid[curr_task])), logical_address,rand()%2)){
                        previous_address[curr_task] = logical_address;
                        break;    /* Page fault */  
                    }
                }
            }
            preempt_task(gtasks, pid[curr_task]);
            context_switches++;
        }
        curr_task=(curr_task+1)%5;
    }
    /* Print overall statistics */
    printf("-------------------------------------\nOverall Statistics:\n\tWorking set sizes at different points:\n\t\t");
    for(int i=0;i<working_set_counts_index;i++){
        printf("%d ",working_set_counts[i]);
    }
    printf("\n\tReferences: %Ld\n\t",references);
    printf("TLB Misses: %Ld (/%Ld)\n\t",tlb_miss,references);
    printf("L1 Total Misses: %Ld (/%Ld)\n\t",l1_read_miss+l1_write_miss, l1_read_access+l1_write_access);
    printf("\tRead Miss:%Ld (/%Ld)\n\t\tWrite Miss:%Ld (/%Ld)\n\t",l1_read_miss,l1_read_access,l1_write_miss,l1_write_access);
    printf("L2 Miss: %Ld (/%Ld)\n\t",l2_miss, l1_read_miss+l1_write_miss);
    printf("Page faults for pages: %Ld (/%Ld)\n\tPage Faults for page tables: %Ld\n\t",page_fault,references,page_fault_pt);
    printf("Page replacements: %Ld\n\t",page_replacements);
    printf("No of times thrashing occured: %d\n\t",frequency_thrashing);
    printf("Context Switches: %Ld\n",context_switches);
    printf("Simulation Done!\n");
}

// frame_table_entry* free_frames_list;
// Free