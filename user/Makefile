CC=g++
INC_PATH=-I. -I.. -I/usr/include/libevdev-1.0
LIB_PATH=#-L/home/dev_tools/apr/lib
LIBS=-lm -levdev
CXXFLAGS=-g -Wall -Wextra -O3 -std=c++20

user.o: user.cpp ../user.h ../util/util.h
	$(CC) $(CXXFLAGS) $(INC_PATH) -c -o $@ $< $(LIB_PATH) $(LIBS)

.PHONY: clean
clean:
	rm -f *.o
