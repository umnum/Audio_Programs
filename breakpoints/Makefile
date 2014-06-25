# makefile for building the breakpoints library

OBJS = breakpoints.o
LIB = include/breakpoints.h
CC = gcc
CFLAGS = -c

breakpoints.o: breakpoints.c $(LIB) 
	sudo cp $(LIB) /usr/include/
	$(CC) $(CFLAGS) breakpoints.c 

clean:
	rm -f $(OBJS)
