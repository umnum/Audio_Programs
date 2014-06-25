# makefile for sfpan

OBJS = sfpan.o
POBJS = ../../portsf-master/portsf/ieee80.o ../../portsf-master/portsf/portsf.o 
BOBJS = ../../breakpoints/breakpoints.o
EXE = sfpan
CC = gcc
CFLAGS = -c 
LFLAGS = -lz $(POBJS) $(BOBJS) -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) 

sfpan.o: sfpan.c
	$(CC) $(CFLAGS) sfpan.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
