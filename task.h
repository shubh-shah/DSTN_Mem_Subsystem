#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "util/queue.h"

/* Process Status codes */
#define RUNNING 0
#define SWAPPED_OUT 1 /* needed for working set */
#define READY 2
#define FINISHED 3
#define WAITING 4

typedef struct{
    long long int references;   /* Call to load_store_byte */
    long long int tlb_miss;     /* get_frame_no_tlb */
    long long int l2_miss;      /* After unsucessful Call to read_l1_cache in load_store_byte */
    long long int l1_miss;      /* After unsucessful Call to read_l2_cache in load_store_byte */
    long long int page_fault;   /* Call to do_page_fault */
    long long int page_fault_pt; /* Call to do_page_fault */
    long long int page_replacements;
    long long int max_working_set; 
} statistics;

typedef struct{
    int pid;
    uint32_t* pgd;          /* Highest Page Dir */
    uint32_t ptlr;
    int status;
    int frames_used;
    statistics stat;
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