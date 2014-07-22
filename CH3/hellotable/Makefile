# makefile for hellotable 

OBJS = hellotable.o
EXE = hellotable 
CC = gcc
CFLAGS = -c
LFLAGS = -lportsf -ltinyAudioLib -lportaudio -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build

hellotable.o: hellotable.c
	$(CC) $(CFLAGS) hellotable.c 
