all:
	clang++ -g -O3 main.cpp `llvm-config --cxxflags --ldflags --libs` -o llvm

assemble:
	llc basicIR.ll -o basicIR.s
	clang basicIR.s -o a.out
