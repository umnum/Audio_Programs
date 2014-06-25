#makefile for sf2float
OBJS = sf2float.o 
POBJS = ../../portsf-master/portsf/ieee80.o ../../portsf-master/portsf/portsf.o 
EXE = sf2float
CC = gcc
CFLAGS = -c 
LFLAGS = -lz $(POBJS) -lm

build:	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE) 

sf2float.o:	sf2float.c
	$(CC) $(CFLAGS) sf2float.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
