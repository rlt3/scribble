#include <cstdio>
#include <cstdlib>

#include <sstream>

#include "parse.hpp"

#include "ir.hpp"
#include "irbuilder.hpp"
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

    IRBuilder bump1;
    bump1.push("33");
    bump1.retvoid();

    IRBuilder bump2;
    bump2.push("72");
    bump2.retvoid();

    Procedure p1("foo", 0, bump1.buildFunc("main"));
    Procedure p2("foo", 0, bump2.buildFunc("main"));

    jit.executeProcedure(p1);
    jit.executeProcedure(p2);

    auto stackptr = jit.getStack();
    printf("%lu\n", stackptr[0]);
    printf("%lu\n", stackptr[1]);

    return 0;
}
