# makefile for building the wave library

OBJS = wave.o

LIB = include/wave.h

CC = gcc

CFLAGS = -c

all: libwave.a 

clean:
	-rm -f $(OBJS)

veryclean:
	-rm -f $(OBJS)
	rm -f libwave.a
	sudo rm -f /usr/lib/libwave.a
	sudo rm -f /usr/include/wave.h 

libwave.a: $(OBJS)
	ar -rc libwave.a $(OBJS)
	ranlib libwave.a

install: libwave.a
	sudo cp libwave.a /usr/lib
	sudo cp $(LIB) /usr/include/

wave.o: wave.c 
	sudo cp $(LIB) /usr/include
	$(CC) $(CFLAGS) wave.c
