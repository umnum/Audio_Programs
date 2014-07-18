# makefile for the gtable library

OBJS = gtable.o

LIB = include/gtable.h

CC = gcc

CFLAGS = -c

all: libgtable.a 

clean:
	-rm -f $(OBJS)

veryclean:
	-rm -f $(OBJS)
	rm -f libgtable.a
	sudo rm -f /usr/lib/libgtable.a
	sudo rm -f /usr/include/gtable.h 

libgtable.a: $(OBJS)
	ar -rc libgtable.a $(OBJS)
	ranlib libgtable.a

install: libgtable.a
	sudo cp libgtable.a /usr/lib
	sudo cp $(LIB) /usr/include/

gtable.o: gtable.c 
	sudo cp $(LIB) /usr/include
	$(CC) $(CFLAGS) gtable.c
