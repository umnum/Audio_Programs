# makefile for player 

OBJS = player.o
EXE = player 
CC = gcc
CFLAGS = -c
LFLAGS = -lportsf -ltinyAudioLib -lportaudio 

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build

player.o: player.c
	$(CC) $(CFLAGS) player.c 
