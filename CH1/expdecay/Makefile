# makefile for expdecay

OBJS = expdecay.o
EXE = expdecay
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

expdecay.o: expdecay.c
	$(CC) $(CFLAGS) expdecay.c 

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
