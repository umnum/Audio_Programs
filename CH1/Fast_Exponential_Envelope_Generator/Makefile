# makefile for FEEG

OBJS = FEEG.o
EXE = FEEG
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

FEEG.o: FEEG.c
	$(CC) $(CFLAGS) FEEG.c 

clean:
	-rm -f $(OBJS) $(EXE)
