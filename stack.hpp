#ifndef SCRIBBLE_STACK

#include <assert.h>
#include "definitions.hpp"

/*
 * Byte-addressable stack implementation.
 */

class Stack
{
public:
    Stack ()
    {
        stack_idx = 0;
        stack_size = 1024;
        stack = (Data*) malloc(sizeof(stack_size * sizeof(Data)));
        if (stack == NULL)
            fatal("Out of memory!");
    }

    void
    push (Data data)
    {
        /*
         * We don't grow the stack right now to ensure the stack's memory
         * addresses always remain the same for the duration of the program
         * due to issues with pointer aliasing. We do an assert here, but TODO
         * should be a stack-overflow interrupt.
        */
        if (stack_idx >= stack_size)
            fatal("Push: stack overflow");
        stack[stack_idx] = data;
        stack_idx++;
    }

    Data
    pop ()
    {
        if (stack_idx == 0)
            fatal("Pop: stack underflow");
        stack_idx--;
        Data data = stack[stack_idx];
        return data;
    }

    Data*
    peek (signed num)
    {
        if (stack_idx == 0)
            fatal("Peek: nothing on stack");
        return stack + stack_idx + num - 1;
    }

protected:
    Data* stack;
    unsigned stack_size;
    unsigned stack_idx;
};

#endif
