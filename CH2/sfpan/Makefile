# makefile for sfpan

OBJS = sfpan.o
EXE = sfpan
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lbreakpoints -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS) 

sfpan.o: sfpan.c
	$(CC) $(CFLAGS) sfpan.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
