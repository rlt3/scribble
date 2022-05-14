#ifndef SCRIBBLE_INTERPRET
#define SCRIBBLE_INTERPRET

#include "machine.hpp"
#include "expression.hpp"
#include <functional>

typedef void (*PrimitiveHandler) ();

/*
 * The Interpret class takes a list of Tokens as input. Tokens are parsed as
 * lists with children. The Interpret class then interprets the tokens and
 * generates an Expression. An Expression is just an evaluated list of tokens
 * and has any symbols and primitives (integers, strings) evaluated. Thus an
 * Expression is just a symbol, representating a procedure, with some number of
 * arguments for that procedure.
 * Applying, or executing, the Expression actually "runs the code", so to
 * speak.
 *
 * For defining symbols in the runtime itself: simply don't apply the
 * Expression and instead define some symbol to be that Expression. E.g:
 *
 *      define(double (x)
 *          add(x x))
 *
 * After parsing the above, one should have a token list like:
 *
 *      - define
 *          - double
 *              - anonymous
 *                  - x
 *              - add
 *                  - x
 *                  - x
 *
 * And then perhaps after interpreting, we get an expression such as:
 *
 *      - define
 *          - double
 *              - move x into reg1 
 *              - push reg1 stack 
 *              - push reg1 stack 
 *              - add
 *
 * Every new definition (made in the code itself) would then just have the
 * symbol point to the instructions. Special symbols, like internal or
 * primitives procedures, may be called like a lambda function below.
 *
 * Ideally, we'd have a basic IR. I think this is what Expression can be. A
 * list of Expressions to be executed is nothing more than an already parsed
 * bytecode. With the Machine providing ample instructions, e.g. `add', `push',
 * `move', we can build a proper language library by hand-writing the bytecode
 * for these primitives as a list of Expressions.
 *
 * source code -> parser      -> tokens
 * tokens      -> compiler    -> bytecode
 * bytecode    -> interpreter -> computation
 *
 * The machine in this scenario represents the target of the compiler. Our
 * machine defines the ISA. The runtime is a combination of all three --
 * parser, compiler, interpreter -- because of the REPL environment and the
 * self-morphing aspect of symbols.
 *
 * We want to be able to store the definitions of symbols within the machine
 * itself so we want to made the Expressions (compiled code) first-class
 * values. We can define areas of the stack for the runtime's use, much like
 * Lua. Perhaps the bottom of the stack holds globals and what not. We could
 * lookup the symbol and evaluate its associated Expressions there. Doing it
 * this way lets us define symbols like `define' as Expressions (like IR, but
 * without designing a textual interface for the IR -- just the AST of IR).
 *
 *
 *
 * An aside: we can use registers for passing arguments to functions just like
 * x86.
 */

class Interpret
{
public:
   /*
    * The 'standard library' is defined below. It uses any existing machine
    * primitives to define procedures. Procedure definitions are cached by the
    * interpreter so symbols may be looked up after they are defined. This also
    * means symbols can be redefined.
    */
    Interpret ()
    {
        /* 
         * define symbols from within the runtime
         */
        symbols["define"] = [](Machine& machine) {
        };

        /* 
         * print the top of the stack.
         */
        symbols["print"] = [](Machine& machine) {
             machine.print(0);
        };

        /* 
         * add the top two values on the stack, placing the result on the
         * stack.
         */
        symbols["add"] = [](Machine& machine) {
             machine.pop(REG1);
             machine.pop(REG2);
             Data d1 = machine.reg(REG1);
             Data d2 = machine.reg(REG2);
             
             unsigned long a = (unsigned long) d1;
             unsigned long b = (unsigned long) d2;
             unsigned long c = a + b;

             machine.move((Data) c, REG1);
             machine.push(REG1);
         };

    }

    void
    expression (Expression& expr)
    {
        if (expr.children().size() > 0) {
            for (auto child : expr.children())
                expression(child);
        }

        switch (expr.type()) {
            case EXPR_PRIMITIVE:
            {
                machine.move(expr.integer(), REG1);
                machine.push(REG1);
                break;
            }

            case EXPR_SYMBOL:
                symbols[expr.string()](machine);
                break;

            case EXPR_LIST:
            default:
                fatal("Unhandled expression type!");
        }
    }

protected:
    Machine machine;
    std::map<std::string, std::function<void (Machine&)>> symbols;

private:
};

#endif
