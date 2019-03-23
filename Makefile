CFLAGS = -Wall -pedantic -std=c99

.DEFAULT_GOAL := all

.PHONY: all debug clean

debug: CFLAGS += -g
debug: clean all

clean:
	rm -rf *.o fitz

fitz.o: fitz.c
	gcc $(CFLAGS) -c fitz.c -o fitz.o

fitz: fitz.o
	gcc $(CFLAGS) fitz.o -o fitz

all: fitz
