#makefile for sf2float
OBJS = sf2float.o 
EXE = sf2float
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lm

build:	$(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

sf2float.o:	sf2float.c
	$(CC) $(CFLAGS) sf2float.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
