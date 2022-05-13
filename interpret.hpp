#ifndef SCRIBBLE_INTERPRET
#define SCRIBBLE_INTERPRET

#include "machine.hpp"
#include "expression.hpp"
#include <functional>

typedef void (*PrimitiveHandler) ();

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
             Data d1 = machine.read(REG1);
             Data d2 = machine.read(REG2);
             
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
