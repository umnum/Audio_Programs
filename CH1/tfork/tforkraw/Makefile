# makefile for tforkraw

OBJS = tforkraw.o
EXE = tforkraw
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

tforkraw.o: tforkraw.c
	$(CC) $(CFLAGS) tforkraw.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
