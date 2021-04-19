MODULES = util mm tlb cache

all: memory_subsystem.o task.o subdirs
	gcc -o driver driver.c memory_subsystem.o mm/paging.o mm/swap.o mm/main_memory.o util/queue.o tlb/tlb.o cache/l1_cache.o cache/l2_cache.o task.o -lpthread
memory_subsystem.o: memory_subsystem.c memory_subsystem.h mem_struct.h
	gcc -c memory_subsystem.c
task.o: task.c task.h mm/main_memory.h global_variables.h util/queue.h
	gcc -c task.c
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