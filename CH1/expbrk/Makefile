# makefile for expbrk

OBJS = expbrk.o
EXE = expbrk
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS) 
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

expbrk.o: expbrk.c
	$(CC) $(CFLAGS) expbrk.c 

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
