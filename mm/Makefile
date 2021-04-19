all: main_memory.o swap.o paging.o
main_memory.o: main_memory.c main_memory.h
	gcc -c main_memory.c
swap.o: swap.c main_memory.h
	gcc -c swap.c
paging.o: paging.c paging.h
	gcc -c paging.c
clean:
	rm *.o *.out