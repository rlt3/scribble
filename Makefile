CFLAGS=-Wall -g -ggdb --std=c++11 -Isrc/ -Isrc/llvm/
LDFLAGS=`llvm-config --cxxflags --ldflags --libs` -rdynamic

all: main

main: src/llvm/llvm.o
	$(CXX) $(CFLAGS) src/main.cpp $^ $(LDFLAGS) -o scribble 

src/llvm/llvm.o: src/llvm/llvm.cpp
	$(CXX) $(CFLAGS) -c src/llvm/llvm.cpp $(LDFLAGS) -o src/llvm/llvm.o

emit:
	clang++ -S -emit-llvm emit.cpp

test: src/llvm/llvm.o
	$(CXX) $(CFLAGS) -Itests/ tests/main.cpp $^ $(LDFLAGS) -o .scribble-test
	./.scribble-test 2>/dev/null
