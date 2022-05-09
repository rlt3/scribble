#ifndef SCRIBBLE_MACHINE

#include <vector>

#include "definitions.hpp"
#include "stack.hpp"

typedef enum {
    REG1 = 0,
    REG2,
    REG3,
    REGCOUNT
} Register;

class Machine
{
public:
    Machine ()
        : stack(Stack())
        , registers(std::vector<Data>(REGCOUNT))
    {
    }

    Data
    reg (Register reg)
    {
        return registers[reg];
    }

    Data
    peek (unsigned idx)
    {
        return *stack.peek(0);
    }

    /*
     * PRIMITIVES
     */

    /* move an immediate value into a register */
    void
    move (unsigned long data, Register reg)
    {
        move((Data) data, reg);
    }

    void
    move (Data data, Register reg)
    {
        registers[reg] = data;
    }

    /* 
     * Load the address of some Data on the stack. `num` is the
     * index of the stack starting from the top and going down.
     * For example, E.g. the top of the stack is 0, second from
     * top is -1, and so on. Load that Data's pointer into a
     * register.
     */
    void
    pointer (signed num, Register reg)
    {
        registers[reg] = (Data) stack.peek(num);
    }

    /*
     * Do the same as above except dereference the pointer as well.
     * This is effectively reading or loading from the stack.
     */
    void
    load (signed num, Register reg)
    {
        registers[reg] = (Data) stack.peek(num);
    }

    /* push a register's value onto the stack */
    void
    push (Register reg)
    {
        stack.push(registers[reg]);
    }

    /* pop from the stack into a register */
    void
    pop (Register reg)
    {
        registers[reg] = stack.pop();
    }

    /* 
     * add the top two values on the stack, placing the result
     * on the stack.
     */
    void
    add ()
    {
        Data d1 = stack.pop();
        Data d2 = stack.pop();
        
        unsigned long a = (unsigned long) d1;
        unsigned long b = (unsigned long) d2;
        unsigned long c = a + b;

        stack.push((void*) c);
    }

protected:
    Stack stack;
    std::vector<Data> registers;
};

#endif
