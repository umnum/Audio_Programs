# makefile for sinetext

OBJS = sinetext.o
EXE = sinetext
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

sinetext.o: sinetext.c
	$(CC) $(CFLAGS) sinetext.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
