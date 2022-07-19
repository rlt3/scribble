CFLAGS=-Wall -g -ggdb --std=c++11 -I. -Illvm/
LDFLAGS=`llvm-config --cxxflags --ldflags --libs` -rdynamic

all: main

main: llvm/llvm.o
	$(CXX) $(CFLAGS) main.cpp $^ $(LDFLAGS) -o scribble 

llvm/llvm.o: llvm/llvm.cpp
	$(CXX) $(CFLAGS) -c llvm/llvm.cpp $(LDFLAGS) -o llvm/llvm.o

test:
	$(CXX) -Wall --std=c++11 -o scribble-test test.cpp
	./scribble-test
	rm ./scribble-test
