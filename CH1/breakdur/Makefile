# makefile for breakdur

OBJS = breakdur.o
EXE = breakdur
CC = gcc
CFLAGS = -c 
LFLAGS = -lbreakpoints 

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

breakdur.o: breakdur.c
	$(CC) $(CFLAGS) breakdur.c 

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
