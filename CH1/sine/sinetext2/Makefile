# makefile for sinetext2

OBJS = sinetext2.o
POBJS = ../../../portsf-master/portsf/ieee80.o ../../../portsf-master/portsf/portsf.o
EXE = sinetext2
CC = gcc
CFLAGS = -c -I ../../../portsf-master/portsf/ -L ../../../portsf-master/include/
LFLAGS = -lz $(POBJS) -lm

build: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE) 

sinetext2.o: sinetext2.c
	$(CC) $(CFLAGS) sinetext2.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
