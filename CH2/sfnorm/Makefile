#makefile for sfnorm
OBJS = sfnorm.o
EXE = sfnorm 
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lm

build:	$(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

sfnorm.o:	sfnorm.c
	$(CC) $(CFLAGS) sfnorm.c

clean:
	-rm -f $(EXE) $(OBJS) 

rebuild: clean build
