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

    IR stack(
        "@stack = global [4096 x i64] zeroinitializer, align 16"
    );

    Procedure p1("foo", 0, IR(
        "@stack = external global [4096 x i64]\n"
        "define i32 @main() {\n"
        "store i64 72, i64* getelementptr inbounds ([4096 x i64], [4096 x i64]* @stack, i64 0, i64 0), align 16\n"
        "\tret i32 72\n"
        "}"
    ));

    Procedure p2("bar", 0, IR(
        "@stack = external global [4096 x i64]\n"
        "define i32 @main() {\n"
        "\tstore i64 33, i64* getelementptr inbounds ([4096 x i64], [4096 x i64]* @stack, i64 0, i64 1), align 8\n"
        "\tret i32 33\n"
        "}"
    ));

    jit.addIR(stack);
    jit.executeProcedure(p1);
    jit.executeProcedure(p2);

    auto stackptr = jit.getStack();
    printf("%lu\n", stackptr[0]);
    printf("%lu\n", stackptr[1]);

    return 0;
}
