all: mem_subsys.o
mem_subsys.o:
	gcc -c memory_subsystem.c
mm:
	$(MAKE) -C mm
clean: 
	rm *.o *.out