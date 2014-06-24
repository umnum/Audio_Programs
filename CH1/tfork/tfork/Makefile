# makefile for tfork

OBJS = tfork.o
EXE = tfork
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

tfork.o: tfork.c
	$(CC) $(CFLAGS) tfork.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
