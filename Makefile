CC=g++
INC_PATH=-I. -I/usr/include/libevdev-1.0
LIB_PATH=#-L/home/dev_tools/apr/lib
LIBS=-lm -lpthread -levdev
CXXFLAGS=-g -Wall -Wextra -O3 -std=c++20
UTILS=$(patsubst %.cpp, %.o, $(wildcard ./util/*.cpp))

.PHONY: all
all: main

main: main.cpp main.h user.h controller.o scancodes.o keyboard.o $(UTILS) user/user.o
	$(CC) $(CXXFLAGS) $(INC_PATH) -o $@ $< controller.o scancodes.o keyboard.o $(UTILS) user/user.o $(LIB_PATH) $(LIBS)

controller.o: controller.cpp controller.h main.h user.h
	$(CC) $(CXXFLAGS) $(INC_PATH) -c -o $@ $< $(LIB_PATH) $(LIBS)

%.o: %.cpp $(wildcard ./util/*.h) user.h
	$(CC) $(CXXFLAGS) $(INC_PATH) -I./util -c -o $@ $< $(LIB_PATH) $(LIBS)

.PHONY: user/user.o
user/user.o:
	$(MAKE) -C user

.PHONY: clean
clean:
	rm -f main *.o util/*.o
	$(MAKE) -C user clean
