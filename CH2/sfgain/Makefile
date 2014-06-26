#makefile for sfgain
OBJS = sfgain.o
EXE = sfgain 
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lm

build:	$(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

sfgain.o:	sfgain.c
	$(CC) $(CFLAGS) sfgain.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
