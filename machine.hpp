#ifndef SCRIBBLE_MACHINE
#define SCRIBBLE_MACHINE

#include <vector>
#include <queue>
#include <map>

#include "definitions.hpp"
#include "error.hpp"
#include "stack.hpp"

struct Procedure
{
    std::string name;
    unsigned long index;

    Procedure()
        : name("NULL")
        , index(0)
    {}

    Procedure(std::string name, unsigned long index)
        : name(name)
        , index(index)
    {}
};

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
        return stack.peek(0);
    }

    /*
     * Write the given instructions to the machine in the reserved part of the
     * stack and define the entry to those instructions as a function.
     */
    unsigned long
    defineProcedure (std::string name, std::queue<Bytecode> instructions)
    {
        unsigned long entry = stack.reserveIndex();
        Data data;

        while (!instructions.empty()) {
            Bytecode bc = instructions.front();
            bc.print();
            data.assign(bc);
            stack.reservePush(data);
            instructions.pop();
        }

        setProcedure(name, entry);
        return entry;
    }

    /*
     * Get the entry point for a symbol.
     */
    unsigned long
    procedureEntry (std::string name)
    {
        auto iter = _definitions.find(name);
        if (iter == _definitions.end())
            fatal("Cannot call undefined symbol `%s'", name.c_str());
        return iter->second.index;
    }


    /*
     * Roll back the stack index for the reserved section. This lets us
     * overwrite definitions. This is useful for executing REPL commands.
     */
    void
    rollbackReserved (unsigned long idx)
    {
        stack.reserveRollback(idx);
    }

    /*
     * Execute the given instructions. The given instructions are treated as
     * coming from a REPL. Thus we add them always to the top of the executable
     * stack and execute them as a procedure.
     */
    void
    execute (std::queue<Bytecode> instructions)
    {
        unsigned long entry = defineProcedure(REPL_SYMBOL, instructions);
        PC = entry;
        registers[REGBASE] = stack.index();

        while (true) {
            Data *data = stack.reserved(PC);
            PC++;

            if (!data->isExecutable())
                fatal("Cannot execute non-executable data at index %d", PC);

            Bytecode bc = data->bytecode();
            switch (bc.op) {

                /* define always halts after defining an expression */
                case OP_HALT:
                    goto cleanup;

                case OP_MOVESTR:
                    move(bc.primitive, bc.reg1);
                    break;

                case OP_MOVE:
                    move(bc.primitive, bc.reg1);
                    break;

                case OP_PUSH:
                    push(bc.reg1);
                    break;

                case OP_POP:
                    pop(bc.reg1);
                    break;

                case OP_PRINT:
                    print();
                    break;

                case OP_ADD:
                    add();
                    break;

                case OP_CALL:
                    call(bc.primitive);
                    break; 

                case OP_RET:
                    ret();
                    break;

                case OP_NULL:
                    fatal("NULL bytecode operator!");
                default:
                    fatal("unimplemented bytecode operator!");
            }
        }

    cleanup:
        stack.reserveRollback(entry);
    }

protected:
    /*
     * Primitives of the machine are defined below. These primitives are best
     * described as assembly instructions.
     */

    /* move an immediate value into a register */
    void
    move (Primitive primitive, Register reg)
    {
        registers[reg].assign(primitive);
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
    }

    /*
     * Do the same as above except dereference the pointer as well.
     * This is effectively reading or loading from the stack.
     */
    void
    load (signed num, Register reg)
    {
        fatal("TODO load");
    }

    /* push a register's value onto the stack */
    void
    push (Register r)
    {
        stack.push(reg(r));
    }

    /* pop from the stack into a register */
    void
    pop (Register reg)
    {
        registers[reg] = stack.pop();
    }

    /* print the value at the given stack index as hex */
    void
    print ()
    {
        Data data = stack.peek(0);
        if (data.isExecutable())
            fatal("Cannot print executable part of stack!");
        data.primitive().print();
    }

    /*
     * Call a procedure by looking up the symbol's entry point and jumping to
     * it.  REGBASE is just the base pointer. This pushes both the return
     * pointer (current PC) and the current REGBASE.
     */
    void
    call (Primitive index)
    {
        Data base = reg(REGBASE);

        ///*
        // * Pop the stack, from bottom to top, into the argument registers up to
        // * the maximum number of argument registers, e.g. if I push 4, 5, and 6
        // * onto the stack, in that order, REG1 = 4, REG2 = 5, etc.
        // */
        //int idx, reg;
        //for (idx = stack.index(), reg = 1;
        //     idx > 0 && reg <= NUM_ARG_REGISTERS;
        //     idx--, reg++)
        //{
        //    registers[reg] = stack.pop();
        //}

        stack.push(Data(PC));
        stack.push(base);
        PC = index.integer();
        registers[REGBASE] = Data(stack.index());
    }

    /*
     * Pops everything off the stack until hitting REGBASE. Pops off the return
     * pointer and old REGBASE.
     */
    void
    ret ()
    {
        unsigned long floor = reg(REGBASE).primitive().integer();
        while (stack.index() > floor)
            stack.pop();
        Data base = stack.pop();
        Data addr = stack.pop();
        PC = addr.primitive().integer();
        registers[REGBASE] = base.primitive().integer();
    }

    void
    add ()
    {
        Data d1 = stack.pop();
        Data d2 = stack.pop();
        stack.push(Data(d1.primitive().integer() + d2.primitive().integer()));
    }

private:
    Stack stack;
    std::vector<Data> registers;
    unsigned long PC; /* program counter */

    std::map<std::string, Procedure> _definitions;

    void
    setProcedure (std::string name, unsigned long entry)
    {
        _definitions[name] = Procedure(name, entry);
        printf("| defined `%s' at %lu\n", name.c_str(), entry);
    }
};

#endif
