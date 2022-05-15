#ifndef SCRIBBLE_BYTECODE
#define SCRIBBLE_BYTECODE

#include "definitions.hpp"

struct Bytecode
{
    Operator op;
    Register reg1;
    Register reg2;
    unsigned long value;

    Bytecode ()
        : op(OP_NULL)
        , reg1(REG1)
        , reg2(REG2)
        , value(0)
    {}

    Bytecode (Operator op, Register r1, Register r2, unsigned long value)
        : op(op)
        , reg1(r1)
        , reg2(r2)
        , value(value)
    {}
};

#endif
