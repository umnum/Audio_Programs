# makefile for envx

OBJS = envx.o
EXE = envx 
CC = gcc
CFLAGS = -c 
LFLAGS = -lportsf -lbreakpoints -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS) 

envx.o: envx.c
	$(CC) $(CFLAGS) envx.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build 
