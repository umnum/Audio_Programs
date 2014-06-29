# makefile for sfenv

OBJS = sfenv.o
EXE = sfenv 
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lbreakpoints -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS) 

sfenv.o: sfenv.c
	$(CC) $(CFLAGS) sfenv.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
