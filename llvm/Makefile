all:
	$(CXX) -g -O3 main.cpp `llvm-config --cxxflags --ldflags --libs` -rdynamic -o llvm

assemble:
	llc basicIR.ll -o basicIR.s
	clang basicIR.s -o a.out
