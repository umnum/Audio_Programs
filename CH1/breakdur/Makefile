# makefile for breakdur

OBJS = breakdur.o
BLIB = ../../breakpoints/breakpoints.o 
EXE = breakdur
CC = gcc
CFLAGS = -c 
LFLAGS = -lz $(BLIB) 

build: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE)

breakdur.o: breakdur.c
	$(CC) $(CFLAGS) breakdur.c 

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
