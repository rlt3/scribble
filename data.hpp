#ifndef SCRIBBLE_DATA
#define SCRIBBLE_DATA

#include "bytecode.hpp"

/*
 * Represents a Word of memory on the stack. But since we have a runtime, we
 * can include stuff like type information.
 */
struct Data
{
    bool isExecutable;
    unsigned long value;
    Bytecode bytecode;

    Data ()
        : isExecutable(false)
        , value(0)
        , bytecode(Bytecode())
    {}

    Data (Bytecode bytecode)
        : isExecutable(true)
        , value(0)
        , bytecode(bytecode)
    {}

    Data (unsigned long value)
        : isExecutable(false)
        , value(value)
        , bytecode(Bytecode())
    {}
};

#endif
