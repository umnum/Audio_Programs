#makefile for portsf
POBJS = ieee80.o portsf.o

PLIB = ../include/portsf.h ieee80.h 

CFLAGS = -c 

CC=gcc

all:	libportsf.a

clean:
	-rm -f $(POBJS)

veryclean:
	-rm -f $(POBJS) 
	rm -f libportsf.a; 
	sudo rm -f /usr/include/portsf.h
	sudo rm -f /usr/include/ieee80.h 

libportsf.a: $(POBJS)  
	ar -rc libportsf.a $(POBJS) 
	ranlib  libportsf.a

install:	libportsf.a
	sudo cp libportsf.a /usr/lib
	sudo cp $(PLIB) /usr/include 
#
#	dependencies
#

ieee80.o: ieee80.c ieee80.h 
	$(CC) -c ieee80.c 

portsf.o: portsf.c portsf.h
	$(CC) -c portsf.c 

portsf.h:
	sudo cp ../include/portsf.h /usr/include
