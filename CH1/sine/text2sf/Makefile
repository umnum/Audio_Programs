# makefile for text2sf

OBJS = text2sf.o
EXE = text2sf
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lm

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS) 

text2sf.o: text2sf.c
	$(CC) $(CFLAGS) text2sf.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
