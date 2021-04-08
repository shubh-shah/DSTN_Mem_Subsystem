#ifndef TASK_H
#define TASK_H

typedef struct{
    int pid;
    uint32_t* pgd;  //Highest Page Dir
    uint32_t ptlr;//Required?
    int status;//READY, WAITING FOR IO
    int frames_used;    //Make this zero
} task_struct;

#endif