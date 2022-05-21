#ifndef SCRIBBLE_BYTECODE
#define SCRIBBLE_BYTECODE

#include "definitions.hpp"
#include "primitive.hpp"

struct Bytecode
{
    Operator op;
    Register reg1;
    Register reg2;
    Primitive primitive;

    Bytecode ()
        : op(OP_NULL)
        , reg1(REG1)
        , reg2(REG2)
        , primitive(Primitive())
    {}

    Bytecode (Operator op, Register r1, Register r2, Primitive primitive)
        : op(op)
        , reg1(r1)
        , reg2(r2)
        , primitive(primitive)
    {}

    Bytecode (Operator op, Register r1, Primitive primitive)
        : op(op)
        , reg1(r1)
        , reg2(REGNULL)
        , primitive(primitive)
    {}

    Bytecode (Operator op, Register r1)
        : op(op)
        , reg1(r1)
        , reg2(REGNULL)
        , primitive(Primitive())
    {}

    Bytecode (Operator op, Primitive primitive)
        : op(op)
        , reg1(REGNULL)
        , reg2(REGNULL)
        , primitive(primitive)
    {}

    Bytecode (Operator op)
        : op(op)
        , reg1(REGNULL)
        , reg2(REGNULL)
        , primitive(Primitive())
    {}
};

#endif
