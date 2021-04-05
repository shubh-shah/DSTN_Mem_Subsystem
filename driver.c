#include <stdio.h>
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
    int done[5]={0};

    memory_subsystem mem;
    int logical_address;
    
    while(!(done[0] && done[1] && done[2] && done[3] && done[4])){
        int time = (rand()%100+150);
        while(time-- && !(done[curr_process] = (fscanf(traces[curr_process],"%x",logical_address)<=0))){
            load_byte(mem,ptbr,ptlr,logical_address);
            // store_byte(mem,ptbr,ptlr,logical_address,data);
        }
        curr_process=(curr_process+1)%5;
    }
}