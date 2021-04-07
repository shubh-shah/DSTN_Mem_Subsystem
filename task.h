#ifndef TASK_H
#define TASK_H

typedef struct{
    int pid;
    int ptbr;
    int ptlr;//Required?
    int status;//READY, WAITING FOR IO
} task_struct;

#endif