# makefile for tabgen

OBJS = tabgen.o
EXE = tabgen
CC = gcc
CFLAGS = -c
LFLAGS = -lportsf -lgtable -lbreakpoints -lm 

build: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build

tabgen.o: tabgen.c
	$(CC) $(CFLAGS) tabgen.c
