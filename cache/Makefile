all: l1_cache.o l2_cache.o
l1_cache.o: l1_cache.c l1_cache.h l2_cache.h
	gcc -c l1_cache.c
l2_cache.o: l2_cache.c l2_cache.h ../util/queue.h
	gcc -c l2_cache.c
clean:
	rm *.o *.out