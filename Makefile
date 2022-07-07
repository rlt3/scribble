all: program

CFLAGS=-Wall -g -ggdb --std=c++11 
LDFLAGS=`llvm-config --cxxflags --ldflags --libs` -rdynamic

program:
	$(CXX) $(CFLAGS) main.cpp $(LDFLAGS) -o scribble 

test:
	$(CXX) -Wall --std=c++11 -o scribble-test test.cpp
	./scribble-test
	rm ./scribble-test
