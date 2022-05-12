all: program

program:
	g++ -Wall --std=c++11 -o scribble main.cpp definitions.cpp
