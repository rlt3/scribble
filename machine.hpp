#ifndef SCRIBBLE_MACHINE
#define SCRIBBLE_MACHINE

#include <vector>
#include <queue>
#include <stack>
#include <map>

#include "definitions.hpp"
#include "error.hpp"
#include "procedure.hpp"
#include "stack.hpp"

class Machine
{
public:
    Machine ()
        : stack(Stack())
        , registers(std::vector<Data>(REGCOUNT))
    {
        /*
         * Define the ancestor procedures for our machine.
         */

        defineProcedure("add", 2, std::queue<Bytecode>({
            Bytecode(OP_ADD),
            Bytecode(OP_RET)
        }));

        defineProcedure("print", 1, std::queue<Bytecode>({
            Bytecode(OP_PRINT),
            Bytecode(OP_RET)
        }));
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
    defineProcedure (std::string name,
                     unsigned long nargs,
                     std::queue<Bytecode> instructions)
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

        setProcedure(name, entry, nargs);
        return entry;
    }

    /*
     * Get the entry point for a symbol.
     */
    unsigned long
    procedureEntry (std::string name)
    {
        return getProcedure(name).entry;
    }

    /*
     * Execute the given instructions. The given instructions are treated as
     * coming from a REPL. Thus we add them always to the top of the executable
     * stack and execute them as a procedure.
     */
    void
    execute (std::queue<Bytecode> instructions)
    {
        unsigned long entry = defineProcedure(REPL_SYMBOL, 0, instructions);

        PC = entry;
        registers[REGBASE] = Data(stack.index());

        while (true) {
            Data *data = stack.reserved(PC);
            PC++;

            if (!data->isExecutable())
                fatal("Cannot execute non-executable data at index %d", PC);

            Bytecode bc = data->bytecode();
            switch (bc.op) {
                case OP_HALT:
                    goto cleanup;

                case OP_MOVESTR:
                    move(bc.primitive, bc.reg1);
                    break;

                case OP_MOVE:
                    move(bc.primitive, bc.reg1);
                    break;

                case OP_LOAD:
                    load(bc.primitive, bc.reg1);
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
     * load a value from the stack into a register. If the signed index is
     * negative then address from the top of the stack, e.g. -1 is the top of
     * the stack, -2 is second from the top, etc. If the index is 0 or positive
     * then we gather arguments from the frame.
     */
    void
    load (Primitive primitive, Register r)
    {
        long index = primitive.integer();
        long whence;
        if (index < 0) {
            whence = index + 1;
        } else {
            long base = reg(REGBASE).primitive().integer();
            whence = base - stack.index() + index + 1;
        }
        registers[r] = stack.peek(whence);
    }

    /* push a register's value onto the stack */
    void
    push (Register r)
    {
        stack.push(reg(r));
    }

    /* pop from the stack into a register */
    void
    pop (Register r)
    {
        /* TODO check stack underflow against REGBASE */
        registers[r] = stack.pop();
    }

    /*
     * Call a procedure by looking up the symbol's entry point and jumping to
     * it.  REGBASE is just the base pointer. This pushes both the return
     * pointer (current PC) and the current REGBASE.
     */
    void
    call (Primitive symbol)
    {
        std::string name = symbol.string();
        auto& proc = getProcedure(name);
        Data old_base = reg(REGBASE);

        if (stack.index() - old_base.primitive().integer() < proc.nargs)
            fatal("Not enough provided arguments for procedure `%s'", name.c_str());

        /* Pop all arguments and hold them temporarily */
        std::stack<Data> arguments;
        for (unsigned long i = 0; i < proc.nargs; i++)
            arguments.push(stack.pop());

        /*
         * Push the return pointer and old base pointer. We do this here, after
         * popping all arguments, so that these arguments will be automatically
         * cleaned up when returning from the call.
         */
        stack.push(Data(PC));
        stack.push(old_base);

        /*
         * Set the new base pointer so that the stack index just before the
         * base of the frame holds the old base pointer and return pointer
         */
        registers[REGBASE] = Data(stack.index());

        /* finally, push all arguments and jump to the procedure */
        while (!arguments.empty()) {
            stack.push(arguments.top());
            arguments.pop();
        }

        PC = proc.entry;
    }

    /*
     * Pops everything off the stack until hitting REGBASE. Uses the return
     * pointer and old base pointer to setup previous stack frame. If there
     * were values on the stack then it treats the top-most value as a return
     * value and places it on top of the previous stack frame.
     */
    void
    ret ()
    {
        Data ret;
        bool has_ret = false;
        unsigned long floor = reg(REGBASE).primitive().integer();

        if (stack.index() > floor) {
            ret = stack.pop();
            has_ret = true;
        }

        while (stack.index() > floor)
            stack.pop();

        Data base = stack.pop();
        Data addr = stack.pop();

        PC = addr.primitive().integer();
        registers[REGBASE] = base.primitive().integer();

        if (has_ret)
            stack.push(ret);
    }

    void
    add ()
    {
        Data d1 = stack.pop();
        Data d2 = stack.pop();
        stack.push(Data(d1.primitive().integer() + d2.primitive().integer()));
    }

    /* print the top most value on the stack */
    void
    print ()
    {
        Data data = stack.peek(0);
        if (data.isExecutable())
            fatal("Cannot print executable part of stack!");
        data.primitive().print();
    }

private:
    Stack stack;
    std::vector<Data> registers;
    unsigned long PC; /* program counter */

    std::map<std::string, Procedure> _definitions;

    void
    setProcedure (std::string name, unsigned long entry, unsigned long nargs)
    {
        _definitions[name] = Procedure(name, entry, nargs);
        printf("| defined `%s' at %lu\n", name.c_str(), entry);
    }

    const Procedure&
    getProcedure (std::string name)
    {
        auto iter = _definitions.find(name);
        if (iter == _definitions.end())
            fatal("Cannot find undefined symbol `%s'", name.c_str());
        return iter->second;
    }
};

#endif
