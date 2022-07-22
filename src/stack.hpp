#ifndef SCRIBBLE_STACK
#define SCRIBBLE_STACK

#include <assert.h>
#include "data.hpp"
#include "error.hpp"

/*
 * Byte-addressable stack implementation.
 *
 * The stack as a reserved area for instructions and a regular push/pop stack
 * for scratch values during execution.
 */

class Stack
{
public:
    Stack ()
    {
        num_reserved = 1024;
        reserved_idx = 0;

        stack_size = 4096;
        stack_idx = num_reserved;

        assert(num_reserved > 0);
        assert(stack_size > num_reserved);

        stack = new Data[stack_size];
    }

    ~Stack ()
    {
        delete[] stack;
    }

    /*
     * Index into the reserved portion of the stack as if it were an array with
     * 0-indexing.
     */
    Data*
    reserved (unsigned long idx)
    {
        return stack + idx;
    }

    unsigned long
    reserveIndex ()
    {
        return reserved_idx;
    }

    void
    reserveRollback (unsigned long idx)
    {
        reserved_idx = idx;
    }

    void
    reservePush (Data data)
    {
        if (reserved_idx >= num_reserved)
            fatal("PushReserved: stack overflow");
        stack[reserved_idx].assign(data);
        reserved_idx++;
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
        stack[stack_idx].assign(data);
        stack_idx++;
    }

    Data
    pop ()
    {
        if (stack_idx == num_reserved)
            fatal("Pop: stack underflow");
        stack[stack_idx].assign(0);
        stack_idx--;
        return stack[stack_idx];
    }

    /*
     * From the top of the stack, relatively address to peek values. The
     * argument should be in the range (-inf, 0]
     */
    Data
    peek (signed long num)
    {
        if (empty())
            fatal("Peek: nothing on stack");
        return stack[stack_idx + num - 1];
    }

    bool
    empty ()
    {
        return (stack_idx == num_reserved);
    }

    unsigned long
    index ()
    {
        return stack_idx;
    }

protected:
    Data* stack;
    unsigned stack_size;
    unsigned stack_idx;
    unsigned num_reserved;
    unsigned reserved_idx;
};

#endif
