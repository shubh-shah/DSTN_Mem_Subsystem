#ifndef TASK_H
#define TASK_H

typedef struct{
    int pid;
    uint32_t ptbr;
    uint32_t ptlr;//Required?
    int status;//READY, WAITING FOR IO
} task_struct;

#endif