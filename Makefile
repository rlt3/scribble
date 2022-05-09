all: program

program:
	g++ --std=c++11 -o scribble main.cpp definitions.cpp
