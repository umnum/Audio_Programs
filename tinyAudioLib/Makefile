# makefile for building the Tiny Audio Library

OBJS = tinyAudioLib.o

LIB = include/tinyAudioLib.h

CC = gcc

CFLAGS = -c

all: libtinyAudioLib.a 

clean:
	-rm -f $(OBJS)

veryclean:
	-rm -f $(OBJS)
	rm -f libtinyAudioLib.a
	sudo rm -f /usr/lib/libtinyAudioLib.a
	sudo rm -f /usr/include/tinyAudioLib.h 

libtinyAudioLib.a: $(OBJS)
	ar -rc libtinyAudioLib.a $(OBJS)
	ranlib libtinyAudioLib.a

install: libtinyAudioLib.a
	sudo cp libtinyAudioLib.a /usr/lib
	sudo cp $(LIB) /usr/include/

tinyAudioLib.o: tinyAudioLib.c 
	sudo cp $(LIB) /usr/include
	$(CC) $(CFLAGS) tinyAudioLib.c -I../portaudio/include 
