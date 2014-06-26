# makefile for building the breakpoints library

OBJS = breakpoints.o

LIB = include/breakpoints.h

CC = gcc

CFLAGS = -c

all: libbreakpoints.a 

clean:
	-rm -f $(OBJS)

veryclean:
	-rm -f $(OBJS)
	rm -f libbreakpoints.a
	sudo rm -f /usr/lib/libbreakpoints.a
	sudo rm -f /usr/include/breakpoints.h 

libbreakpoints.a: $(OBJS)
	ar -rc libbreakpoints.a $(OBJS)
	ranlib libbreakpoints.a

install: libbreakpoints.a
	sudo cp libbreakpoints.a /usr/lib
	sudo cp $(LIB) /usr/include/

breakpoints.o: breakpoints.c 
	sudo cp $(LIB) /usr/include
	$(CC) $(CFLAGS) breakpoints.c
