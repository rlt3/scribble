#ifndef SCRIBBLE_BYTECODE
#define SCRIBBLE_BYTECODE

#include "machine.hpp"

typedef enum {
    OP_NULL,
    OP_MOVE,
    OP_POINTER,
    OP_LOAD,
    OP_PUSH,
    OP_POP,
    OP_PRINT,
} BytecodeOP;

class Bytecode
{
public:
    Bytecode ()
        : op(OP_NULL)
        , reg1(REG1)
        , reg2(REG2)
        , data(0)
    {
    }

    Bytecode (BytecodeOP op, Register r1, Register r2, Data data)
        : op(op)
        , reg1(r1)
        , reg2(r2)
        , data(data)
    {
    }

    void
    execute (Machine &m)
    {
        switch (op) {
            case OP_MOVE:
                m.move(data, reg1);
                break;

            case OP_POINTER:
                fatal("Unimplemened POINTER");
                break;

            case OP_LOAD:
                fatal("Unimplemened LOAD");

            case OP_PUSH:
                m.push(reg1);
                break;

            case OP_POP:
                m.push(reg1);
                break;

            case OP_PRINT:
                m.print((long) data);
                break;

            case OP_NULL:
            default:
                fatal("NULL bytecode operator!");
        }
    }

protected:
    BytecodeOP op;
    Register reg1;
    Register reg2;
    Data data;
};

#endif
