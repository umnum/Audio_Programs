#makefile for sfnorm
OBJS = sfnorm.o
POBJS = ../../portsf-master/portsf/ieee80.o ../../portsf-master/portsf/portsf.o 
EXE = sfnorm 
CC = gcc
CFLAGS = -c -L ../../portsf-master/include/ 
LFLAGS = -lz $(POBJS) -lm

build:	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE) 

sfnorm.o:	sfnorm.c
	$(CC) $(CFLAGS) sfnorm.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
