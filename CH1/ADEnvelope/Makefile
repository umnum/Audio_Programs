#makefile for ADEnvelope 

OBJS = ADEnvelope.o
EXE = ADEnvelope 
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)	
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

ADEnvelope.o:	ADEnvelope.c
	$(CC) $(CFLAGS) ADEnvelope.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
