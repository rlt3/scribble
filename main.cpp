#include <cstdio>
#include <cstdlib>

#include "parse.hpp"
#include "interpret.hpp"
#include "bytecode.hpp"

int
main (int argc, char **argv)
{
    //Machine M;
    //M.move(1300, REG1);
    //M.move(  37, REG2);
    //M.push(REG1);
    //M.push(REG2);
    //M.add();
    //M.print(0);

    /*
     * In this system, the primitives always exist. One can define more symbols
     * which are just collections of primitives. Eventually, symbols will be
     * defined using other symbols and primitives.
     * The above collection could be defined as a symbol, e.g. `PUSH-1337`.
     * Our primitives are defined in such a manner to allow for the user to
     * extrapolate the language to suit their own purpose for that program.
     *
     * The way the program works at a high level:
     *   1) A tiny kernel is defined which has pre-existing primitives.
     *   2) Ancestral symbols are defined which make direct use of these
     *   existing primitives and other ancestral symbols.
     *   3) Descendent symbols are defined entirely in ancestral symbols
     *   without the use of primitives.
     * The language itself necessitates a runtime. The tiny kernel starts and
     * immediate begins parsing/compiling the program which defines its symbols
     * (ancestral & descendent). Then, when all definitions are defined,
     * execution of the main program begins.
     *
     * Because we can define primitives in C but it is compiled into whatever
     * target we define, we can effectively think of the primitives in term of
     * x86 or other assemblies. In this way, we can compile code at runtime --
     * we can compile in a REPL environment -- that just references these
     * already compiled primitives. Our `compiled` code can be very, very
     * simple because we are simply worried about these very small primitives
     * which can simply be jumped to in the worst case, or copy-pasted in the
     * best case. In either, they are fungible.
     *
     *
     * TODO:
     * - simple parser
     *   1) language is executed one line at a time.
     *   2) symbol followed by any number of arguments, separated by spaces.
     *   3) symbols can be defined using a special syntax.
     * - primitive to define new symbols using other symbols.
     * - simple interpreter
     *   1) parser should parse into a list of symbols to execute.
     *   2) executing the list by walking it is OK.
     *   3) executing a symbol may define other symbols in the runtime. the
     *   addresses and definitions of these symbols must be maintained.
     * - executable buffer
     *   1) eventually, would like to `compile` list to native code.
     *   2) because primitives already exist in native code, can take advantage
     *   of this by having pre-compiled glue code which is meant to be patched
     *   with address of primitives.
     *   3) this way, we can walk the parsed list of symbols and compile them
     *   into native code by copy-pasting addresses of primitives and other
     *   symbols once they've been compiled.
     */

    Parse parse(std::cin);
    Interpret interpret;

    auto expr1 = parse.expression();
    expr1.print();
    interpret.expression(expr1);

    auto expr2 = parse.expression();
    expr2.print();
    interpret.expression(expr2);

    return 0;
}
