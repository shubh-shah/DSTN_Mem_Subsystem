# Memory Subsystem (DSTN Assignment 1)
Implementation of complete memory subsystem. 

## Group Members
- Anshul Sood (2017A7PS0939G)
- Shubh Pragnesh Shah (2018A7PS0092G)

## Contributions
- Anshul Sood - Implementation of TLB, L1 Cache, L2 Cache and Queue Utility. (40%)
- Shubh Pragnesh Shah - Implementation of Paging, Main Memory, Thrashing, Memory Subsytem, Driver and overall Integration. (60%)

## Problem Description
The memory subsystem [with TLB, L1 Cache, L2 Cache and Main Memory] has following configuration:
- TLB: Identifier based TLB [with PID stored in each entry of the TLB. Invalidation to corresponding entries happens when a process terminates. You need to take care of shared pages explicitly]. Number of entries in TLB: 20 [Replacement: FIFO]
- L1 Cache: 4KB, 16B, 4 Way set associative phased cache. The cache is Virtually tagged and Physically Indexed. The cache follows Write through and Look through. It follows LRU Square matrix as replacement policy.
- L2 Cache: 32KB, 32B, 8 Way set associative cache. The cache follows Write back and look aside. It follows FIFO as replacement policy.
- Main Memory with Memory Management: 32MB Main memory with LRU as replacement policy. The memory management scheme used is Pure Paging.
- Thrashing mechanism to implement: Working set strategy.

### Given Assumptions
1. Virtual Memory contains the complete program. Hard disk is big enough to hold all the programs and No network file is accessed.
2. First 2 blocks of the process (assume the page size and frame size same and is 512B) will be pre-paged into main memory before a process starts its execution.
3. All other pages are loaded on demand [Assume the system supports dynamic loading. Also assume that the system has no dynamic linking support].
4. Main memory follows Global replacement. Lower limit number of pages and upper limit number of pages per process should be strictly maintained.
5. Page tables [and segment tables wherever applicable] of all the processes reside in main memory and will not be loaded into cache memory levels.

## Project Structure
- [cache](cache) : Implementation of L1 and L2 caches
  - [l1_cache.h](cache/l1_cache.h) & [l1_cache.c](cache/l1_cache.c) : Functions for reading and writing from l1 cache, updating the cache after miss
  - [l2_cache.h](cache/l2_cache.h) & [l2_cache.c](cache/l2_cache.c) : Functions for reading and writing from l2 cache, updating the cache after miss
- [mm](mm) : Implementation of main memory
  - [main_memory.h](mm/main_memory.h) : Contains declarations of struct of main memory and functions relevent to main memory, Definitions of these functions split into two files, swap.c and main_memory.c
  - [main_memory.c](mm/main_memory.c) : Contains definitions of functions for doing page walk, handling page faults, allocating and deallocating frames, handling interrupts for working set updation, and for handling block transfers from caches
  - [swap.c](mm/swap.c) : Contains definitions of functions required for swapping frames in and out of memory, and unloading all frames of a task
  - [paging.h](mm/paging.h) : Contains macros, structures and declarations of functions relevent to paging (i.e macros essential for page table walk, struct for frame table and functions for it's maintainance)
  - [paging.c](mm/paging.c) : Contains functions that maintain the state of frame table and LRU queue
- [tlb](tlb) : Implementation of TLB
  - [tlb.h](tlb/tlb.h) : Struct definition for TLB and Functions for maintaing TLB, like get entry, insert entry, invalidate tlb (TLB flush) 
  - [tlb.c](tlb/tlb.c) : Function definitions
- [util](util) : Implementation of Queue data structure
  - [queue.h](util/queue.h) & [queue.c](util/queue.c) : Structure and basic functions like push, pop, is_empty
- [mem_struct.h](mem_struct.h) : Definition of structure for the whole memory subsystem, containing tlb, l1 cache, l2 cache, and mm.
- [memory_subsystem.h](memory_subsystem.h) & [memory_subsystem.h](memory_subsystem.h) : Definition of high level functions like load or store byte which calls relevent functions from mm, tlb, caches to do so.
- [task.c](task.c) & [task.h](task.h): Definition of data structure for a task, containing relevent fields like base of the highest page directory, PID, status and various other statistics. Functions for running a task, destroying a task after completion, finding a task from PID etc are also provided
- [global_variables.h](global_variables.h) : Declaration of a few global variables required, mostly for statistics and handling of working set interrupt as we cannot pass variables to it.
- [driver.c](driver.c) : Definition of main function that calls the relevent functions from memory_subsystem and task

## How to Compile and Run:
- Call "make clean" and "make cleans" to clean previously compiled files
- Call "make all"
- Call ./driver

## Description
The following sequence is followed:
1. Tasks are initialised using init_task at the begining which creates a pointer to Page global directory, when task is finished, it is destroyed (Pages and Page tables unallocated and other book keeping). 
2. All accesses go through the high level function load_store_byte (or the two compainion macros for load_byte and store_byte). This function calls the TLB for the frame number, if TLB miss, calls do_page_table_walk which resolves the frame no, puts it in TLB and then instruction is restarted. 
3. If a page fault occurs, do_page_table_walk gets a frame and return an error - this leads to a context switch as processor is given to the next available task. 
4. After frame number is recieved, If it is a load instruction, L1 cache is called, if L1 miss occurs, we call both L2 cache and memory for data (as L2 is look aside). L1 and L2 caches are then updated if misses have occured and L1 cache is read again. 
5. If instruction is store, we call L1, if a miss occurs, we follow write allocate strategy, this triggers same actions as a read miss in L1 and data is brought in from lower levels. After this, data is writeen into L1 and L2 caches simultaneously as it is write through and writes to memory occur only during replacement of blocks from L2 cache.

## Bugs?
None we are aware of.