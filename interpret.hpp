#ifndef SCRIBBLE_INTERPRET
#define SCRIBBLE_INTERPRET

#include "machine.hpp"
#include "expression.hpp"

class Interpret
{
public:
    Interpret ()
    {
    }

    void
    expression (Expression& expr)
    {
        expr.print();
    }


protected:
    Machine machine;

private:
};

#endif
