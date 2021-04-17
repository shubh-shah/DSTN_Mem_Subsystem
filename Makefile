MODULES = ADT mm tlb

all: memory_subsystem.o subdirs
	gcc -o driver driver.c memory_subsystem.o mm/paging.o mm/swap.o mm/main_memory.o ADT/queue.o tlb/tlb.o -lpthread
memory_subsystem.o:
	gcc -c memory_subsystem.c -Wall
subdirs:
	for dir in $(MODULES); do \
		(cd $$dir; ${MAKE} all); \
	done
clean:
	rm *.o *.out
cleans:
	for dir in $(MODULES); do \
		(cd $$dir; ${MAKE} clean); \
	done