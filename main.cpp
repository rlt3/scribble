#include <cstdio>
#include <cstdlib>

#include <sstream>

#include "parse.hpp"

#include "ir.hpp"
#include "procedure.hpp"
#include "jit.hpp"

int
main (int argc, char **argv)
{
    //std::queue<Token> tokens;
    //Parse parse(std::cin);
    //tokens = parse.stream();
    
    JIT::preinitialize();
    JIT jit;

    Procedure proc("foo", 0, IR(
        "define i32 @main() {\n"
        "\tret i32 72\n"
        "}"
    ));

    //outs() << "ir: " << proc.getIRString() << "\n";

    jit.executeProcedure(proc);

    return 0;
}
