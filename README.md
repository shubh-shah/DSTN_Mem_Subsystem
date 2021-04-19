# Memory Subsystem (DSTN Assignment 1)
Implementation of complete memory subsystem. 

## Group Members
- Anshul Sood
- Shubh Pragnesh Shah 2018A7PS0092G   

## Project Structure
- [cache](cache) : Implementation of L1 and L2 caches
  - l1_cache.h & l1_cache.c
  - l2_cache.h & l2_cache.c
- [mm](mm) : Implementation of main memory
  - [main_memory.h](mm/main_memory.h) : Contains declarations of struct of main memory and functions relevent to main memory, Definitions of these functions split into two files, swap.c and main_memory.c
  - [main_memory.c](mm/main_memory.c) : Contains definitions of functions for doing page walk, handling page faults, allocating and deallocating frames, handling interrupts for working set updation, and for handling block transfers from caches
  - [swap.c](mm/swap.c) : Contains definitions of functions required for swapping frames in and out of memory, and unloading all frames of a task
  - [paging.h](mm/paging.h) : Contains macros, structures and declarations of functions relevent to paging (i.e macros essential for page table walk, struct for frame table and functions for it's maintainance)
  - [paging.c](mm/paging.c) : Contains functions that maintain the state of frame table and LRU queue
- [tlb](tlb) : Implementation of TLB
  - [tlb.h](tlb.h) : Struct definition for TLB and Functions for maintaing TLB, like get entry, insert entry, invalidate tlb (TLB flush) 
  - [tlb.c](tlb.c) : Function definitions
- [util](util) : Implementation of Queue data structure
  - [queue.h](util/queue.h) & [queue.c](queue.c) : Structure and basic functions like push, pop, is_empty
- [mem_struct.h](mem_struct.h) : Definition of structure for the whole memory subsystem, containing tlb, l1 cache, l2 cache, and mm.
- [memory_subsystem.h](memory_subsystem.h) & [memory_subsystem.h](memory_subsystem.h) : Definition of high level functions like load or store byte
- [task.c](task.c) & [task.h](task.h): Definition of data structure for a task, containing relevent fields like base of the highest page directory, PID, status and various other statistics. Functions for running a task, destroying a task after completion, finding a task from PID etc are also provided
- [global_variables.h](global_variables.h) : Declaration of a few global variables required, mostly for statistics and handling of working set interrupt as we cannot pass variables to it.
- [driver.c](driver.c) : Definition of main function that calls the relevent functions from memory_subsystem and task

## How to Compile and Run:
- Call "make clean" and "make cleans" to clean previously compiled files
- Call "make all"
- Call ./driver

## Description of structure

## Bugs?
