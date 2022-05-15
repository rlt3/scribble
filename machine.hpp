#ifndef SCRIBBLE_MACHINE
#define SCRIBBLE_MACHINE

#include <vector>
#include <queue>

#include "definitions.hpp"
#include "data.hpp"
#include "stack.hpp"
#include "bytecode.hpp"

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
     * Write the given instructions to the machine in the reserved part of the
     * stack. Returns the idx just after writing the last instruction.
     */
    unsigned long
    write (unsigned long idx, std::queue<Bytecode> instructions)
    {
        for (; !instructions.empty(); idx++) {
            Data *data = stack.reserved(idx);
            data->isExecutable = true;
            data->bytecode = instructions.front();
            instructions.pop();
        }
        return idx;
    }

    /*
     * Execute starting from the given index on the stack, starting from the
     * bottom and moving up.
     */
    void
    execute (unsigned long idx)
    {
        Data *data = stack.reserved(idx);

        if (!data->isExecutable)
            fatal("Cannot execute non-executable data at index %d", idx);

        Bytecode bc = data->bytecode;
        switch (bc.op) {
            case OP_HALT:
                return;

            case OP_MOVE:
                move(bc.value, bc.reg1);
                break;

            case OP_POINTER:
                fatal("Unimplemened POINTER");
                break;

            case OP_LOAD:
                fatal("Unimplemened LOAD");

            case OP_PUSH:
                push(bc.reg1);
                break;

            case OP_POP:
                push(bc.reg1);
                break;

            case OP_PRINT:
                print((long) bc.value);
                break;

            case OP_ADD:
                add();
                break;

            case OP_NULL:
            default:
                fatal("NULL bytecode operator!");
        }

        execute(idx + 1);
    }

protected:
    /*
     * Primitives of the machine are defined below. These primitives are best
     * described as assembly instructions.
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
        fatal("TODO pointer");
        //registers[reg] = (Data) stack.peek(num);
    }

    /*
     * Do the same as above except dereference the pointer as well.
     * This is effectively reading or loading from the stack.
     */
    void
    load (signed num, Register reg)
    {
        fatal("TODO load");
        //registers[reg] = (Data) stack.peek(num);
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

    /* print the value at the given stack index as hex */
    void
    print (long idx)
    {
        Data data = peek(idx);
        if (data.isExecutable)
            fatal("Cannot print executable part of stack!");
        printf("0x%08lx\n", data.value);
    }

    void
    add ()
    {
         Data d1 = stack.pop();
         Data d2 = stack.pop();
         stack.push(Data(d1.value + d2.value));
     }

protected:
    Stack stack;
    std::vector<Data> registers;
};

#endif
