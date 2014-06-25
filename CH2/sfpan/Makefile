# makefile for sfpan

OBJS = sfpan.o
POBJS = ../../portsf-master/portsf/ieee80.o ../../portsf-master/portsf/portsf.o 
EXE = sfpan
CC = gcc
CFLAGS = -c -I ../../portsf-master/portsf/ -L ../../portsf-master/include/ 
LFLAGS = -lz $(POBJS) -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) 

sfpan.o: sfpan.c
	$(CC) $(CFLAGS) sfpan.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
