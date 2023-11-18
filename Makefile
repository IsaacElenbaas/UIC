CC=g++
INC_PATH=-I. -I/usr/include/libevdev-1.0
LIB_PATH=#-L/home/dev_tools/apr/lib
LIBS=-lm -lpthread -levdev
CXXFLAGS=-g -Wall -Wextra -O3 -std=c++20

.PHONY: all
all: main

main: main.cpp main.h controller.o
	$(CC) $(CXXFLAGS) $(INC_PATH) -o $@ $< controller.o $(LIB_PATH) $(LIBS)

controller.o: controller.cpp controller.h main.h
	$(CC) $(CXXFLAGS) $(INC_PATH) -c -o $@ $< $(LIB_PATH) $(LIBS)

.PHONY: clean
clean:
	rm -f main *.o
