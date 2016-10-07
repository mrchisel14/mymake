
CC=g++
CFLAGS=-I. -ansi -pedantic -Wall
COFLAGS=-c -I. -ansi -pedantic -Wall

mymake.x: main.o
	$(CC) $(CFLAGS) -o mymake.x main.o 
clean:
	rm -f *.o *.x
