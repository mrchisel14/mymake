#Entire line comment
CC=g++ #After Macro
CFLAGS=-I. -ansi -pedantic -Wall -std=c++11
COFLAGS=-c -I. -ansi -pedantic -Wall -std=c++11

mymake.x: main.o  make_data.o static_utilities.o
	$(CC) $(CFLAGS) -o mymake.x main.o static_utilities.o make_data.o
main.o: main.cpp make_data.h static_utilities.h
	$(CC) $(COFLAGS) -o main.o -c main.cpp
static_utilities.o: static_utilities.cpp make_data.h static_utilities.h
	$(CC) $(COFLAGS) -o static_utilities.o -c static_utilities.cpp
make_data.o: make_data.cpp make_data.h
	$(CC) $(COFLAGS) -o make_data.o -c make_data.cpp
clean:
	rm -f *.o *.x
