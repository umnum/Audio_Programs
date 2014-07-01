# makefile for sinetext2

OBJS = sinetext2.o
EXE = sinetext2
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

sinetext2.o: sinetext2.c
	$(CC) $(CFLAGS) sinetext2.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
