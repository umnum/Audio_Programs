#makefile for sfgain
OBJS = sfgain.o
POBJS = ../../portsf-master/portsf/ieee80.o ../../portsf-master/portsf/portsf.o 
EXE = sfgain 
CC = gcc
CFLAGS = -c 
LFLAGS = -lz $(POBJS) -lm

build:	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE) 

sfgain.o:	sfgain.c
	$(CC) $(CFLAGS) sfgain.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
