# makefile for siggen

OBJS = siggen.o
EXE = siggen
CC = gcc
CFLAGS = -c
LFLAGS = -lportsf -lwave -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build

siggen.o: siggen.c
	$(CC) $(CFLAGS) siggen.c 
