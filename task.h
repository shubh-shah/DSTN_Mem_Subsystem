#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "ADT/queue.h"

/* Process Status codes */
#define RUNNING 0
#define SWAPPED_OUT 1 /* needed for working set */
#define READY 2
#define FINISHED 3
#define WAITING 4

typedef struct{
    int pid;
    uint32_t* pgd;          /* Highest Page Dir */
    uint32_t ptlr;          // Required?
    int status;             
    int frames_used;        //Make this zero
} task_struct;

typedef struct{
    queue* list;
    int next_pid;
} task_list;

extern task_list* init_task_list();

extern int init_task();
extern task_struct* find_task(int pid);
extern bool run_task(int pid);
extern bool preempt_task(int pid);
extern bool destroy_task(int pid);

#endif