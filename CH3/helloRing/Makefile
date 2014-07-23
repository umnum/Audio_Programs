# makefile for helloRing 

OBJS = helloRing.o
EXE = helloRing 
CC = gcc
CFLAGS = -c
LFLAGS = -lportaudio -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build

helloRing.o: helloRing.c
	$(CC) $(CFLAGS) helloRing.c 
