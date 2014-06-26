#makefile for sfprop 
OBJS = sfprop.o
EXE = sfprop 
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lm

build:	$(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

sfprop.o:	sfprop.c
	$(CC) $(CFLAGS) sfprop.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
