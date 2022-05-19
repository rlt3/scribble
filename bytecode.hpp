#ifndef SCRIBBLE_BYTECODE
#define SCRIBBLE_BYTECODE

#include "definitions.hpp"
#include "data.hpp"

struct Bytecode
{
    Operator op;
    Register reg1;
    Register reg2;
    Data data;

    Bytecode ()
        : op(OP_NULL)
        , reg1(REG1)
        , reg2(REG2)
        , data(Data(0))
    {}

    Bytecode (Operator op, Register r1, Register r2, unsigned long value)
        : op(op)
        , reg1(r1)
        , reg2(r2)
        , data(Data(value))
    {}

    Bytecode (Operator op, Register r1, unsigned long value)
        : op(op)
        , reg1(r1)
        , reg2(REGNULL)
        , data(Data(value))
    {}

    Bytecode (Operator op, Register r1, std::string str)
        : op(op)
        , reg1(r1)
        , reg2(REGNULL)
        , data(Data(str))
    {}

    Bytecode (Operator op, Register r1)
        : op(op)
        , reg1(r1)
        , reg2(REGNULL)
        , data(Data(0))
    {}

    Bytecode (Operator op, unsigned long val)
        : op(op)
        , reg1(REGNULL)
        , reg2(REGNULL)
        , data(Data(val))
    {}

    Bytecode (Operator op)
        : op(op)
        , reg1(REGNULL)
        , reg2(REGNULL)
        , data(Data(0))
    {}
};

#endif
