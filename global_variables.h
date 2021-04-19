#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include "task.h"
#include "mem_struct.h"

/* Global variables required */
extern task_list* gtasks;
extern memory_subsystem* gm_subsys;
extern int working_set_counts[100];
extern int working_set_counts_index;
extern int frequency_thrashing;

#endif