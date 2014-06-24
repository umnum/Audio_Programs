# makefile for text2sf

OBJS = text2sf.o
POBJS = ../../../portsf-master/portsf/ieee80.o ../../../portsf-master/portsf/portsf.o
EXE = text2sf
CC = gcc
CFLAGS = -c -I ../../../portsf-master/portsf/ -L ../../../portsf-master/include/
LFLAGS = -lz $(POBJS) -lm

build: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE) 

text2sf.o: text2sf.c
	$(CC) $(CFLAGS) text2sf.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
