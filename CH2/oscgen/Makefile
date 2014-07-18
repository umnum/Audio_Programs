# makefile for oscgen

OBJS = oscgen.o
EXE = oscgen
CC = gcc
CFLAGS = -c

LFLAGS = -lportsf -lbreakpoints -lwave -lgtable -lm

build: $(OBJS) 
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 

oscgen.o: oscgen.c
	$(CC) $(CFLAGS) oscgen.c 
