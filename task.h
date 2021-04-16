#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define RUNNING 0
#define SWAPPED_OUT 1 //- needed for working set
#define READY 2
#define FINISHED 3
#define WAITING 4

typedef struct{
    uint8_t* main_mem_base; //Temporary, for working set handling - make this global afterwards uint32_t pgd_frame_no;
    int pid;
    uint32_t* pgd;          /* Highest Page Dir */
    uint32_t ptlr;          // Required?
    int status;             //READY, WAITING FOR IO
    int frames_used;        //Make this zero
} task_struct;


//Global variables required
extern task_struct tasks[1024];  //Array
extern int no_of_tasks;     //Maintained by init_task

#endif