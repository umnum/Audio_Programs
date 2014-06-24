# makefile for breakdur

OBJS = breakdur.o
EXE = breakdur
CC = gcc
CFLAGS = -c

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE)

breakdur.o: breakdur.c
	$(CC) $(CFLAGS) breakdur.c 

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
