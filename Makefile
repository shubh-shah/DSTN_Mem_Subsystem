MODULES = ADT mm tlb cache

all: memory_subsystem.o subdirs
	gcc -o driver driver.c memory_subsystem.o mm/paging.o mm/swap.o mm/main_memory.o ADT/queue.o tlb/tlb.o cache/l1_cache.o cache/l2_cache.o -lpthread
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