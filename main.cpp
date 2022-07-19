#include <cstdio>
#include <cstdlib>

#include <sstream>

#include "parse.hpp"

#include "ir.hpp"
#include "irbuilder.hpp"
#include "procedure.hpp"
#include "runtime.hpp"

int
main (int argc, char **argv)
{
    //std::queue<Token> tokens;
    //Parse parse(std::cin);
    //tokens = parse.stream();
    
    Runtime runtime;

    IRBuilder bump1;
    bump1.push("33");
    bump1.retvoid();

    IRBuilder bump2;
    bump2.push("72");
    bump2.push("9");
    bump2.retvoid();

    Procedure p1("foo", 0, bump1.buildFunc("foo"));
    Procedure p2("foo", 0, bump2.buildFunc("foo"));

    runtime.executeProcedure(p1);
    runtime.executeProcedure(p2);

    auto stackptr = runtime.getStack();
    printf("%lu\n", stackptr[0]);
    printf("%lu\n", stackptr[1]);

    auto typestack = runtime.getTypestack();
    printf("%u\n", typestack.top()); typestack.pop();
    printf("%u\n", typestack.top());

    return 0;
}
