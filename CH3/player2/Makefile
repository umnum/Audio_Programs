# makefile for player2 

OBJS = player2.o
EXE = player2 
CC = gcc
CFLAGS = -c
LFLAGS = -lportsf -ltinyAudioLib -lportaudio -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build

player2.o: player2.c
	$(CC) $(CFLAGS) player2.c 
