all: program

program:
	g++ -Wall -g -ggdb --std=c++11 -o scribble main.cpp

test:
	g++ -Wall --std=c++11 -o scribble-test test.cpp
	./scribble-test
	rm ./scribble-test
