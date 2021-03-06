#Entire line comment
CC=g++ #After Macro
CCC=g++
CFLAGS=-I.
COFLAGS=-I. -ansi -pedantic -Wall -std=c++11
CCCOFLAGS=-I. -ansi -pedantic -Wall -std=c++11

mymake.x: main.o  make_data.o static_utilities.o exec_handler.o shell_tools.o internal_commands.o procQueue.o
	$(CC) $(CFLAGS) -pthread -o mymake.x main.o static_utilities.o make_data.o exec_handler.o shell_tools.o internal_commands.o procQueue.o
main.o: main.cpp make_data.h static_utilities.h
	$(CC) $(COFLAGS) -o main.o -c main.cpp
static_utilities.o: static_utilities.cpp make_data.h static_utilities.h
	$(CC) $(COFLAGS) -o static_utilities.o -c static_utilities.cpp
make_data.o: make_data.cpp make_data.h exec_handler.h
	$(CC) $(COFLAGS) -o make_data.o -c make_data.cpp
exec_handler.o: exec_handler.cpp exec_handler.h shell_tools.h internal_commands.h procQueue.h
	$(CC) $(COFLAGS) -o exec_handler.o -c exec_handler.cpp
shell_tools.o: shell_tools.cpp shell_tools.h
	$(CCC) $(CCCOFLAGS) -o shell_tools.o -c shell_tools.cpp
internal_commands.o: internal_commands.c internal_commands.h shell_tools.h
	$(CCC) $(CCCOFLAGS) -o internal_commands.o -c internal_commands.c
procQueue.o: procQueue.c procQueue.h shell_tools.h
	$(CCC) $(CCCOFLAGS) -o procQueue.o -c procQueue.c

clean:
	rm -f *.o *.x
