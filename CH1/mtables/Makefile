# makefile for mtables

OBJS = mtables.o
EXE = mtables
CC = gcc
CFLAGS = -c

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE)

mtables.o: mtables.c
	$(CC) $(CFLAGS) mtables.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
