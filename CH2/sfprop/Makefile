#makefile for sfprop 
OBJS = sfprop.o
POBJS = ../../portsf-master/portsf/ieee80.o ../../portsf-master/portsf/portsf.o 
EXE = sfprop 
CC = gcc
CFLAGS = -c -L ../../portsf-master/include/ 
LFLAGS = -lz $(POBJS) -lm

build:	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE) 

sfprop.o:	sfprop.c
	$(CC) $(CFLAGS) sfprop.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
