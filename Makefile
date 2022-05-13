all: program

program:
	g++ -Wall --std=c++11 -o scribble main.cpp definitions.cpp

test:
	g++ -Wall --std=c++11 -o scribble-test test.cpp
	./scribble-test
	rm ./scribble-test
