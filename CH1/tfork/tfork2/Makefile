# makefile for tfork2

OBJS = tfork2.o
EXE = tfork2
CC = gcc
CFLAGS = -c
LFLAGS = -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

tfork2.o: tfork2.c
	$(CC) $(CFLAGS) tfork2.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
