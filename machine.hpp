#ifndef SCRIBBLE_MACHINE
#define SCRIBBLE_MACHINE

#include <vector>
#include <queue>

#include "definitions.hpp"
#include "data.hpp"
#include "stack.hpp"
#include "bytecode.hpp"

struct Definition
{
    std::string name;
    unsigned long index;
    //unsigned long length;

    Definition()
        : name("NULL")
        , index(0)
     //   , length(0)
    {}

    //Definition(std::string name, unsigned long index, unsigned long length)
    Definition(std::string name, unsigned long index)
        : name(name)
        , index(index)
      //  , length(length)
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
        return *stack.peek(0);
    }

    /*
     * Write the given instructions to the machine in the reserved part of the
     * stack and define the entry to those instructions as a function.
     */
    void
    defineBytecode (std::string name, std::queue<Bytecode> instructions)
    {
        unsigned long entry = stack.reservedTop();

        while (!instructions.empty()) {
            stack.pushReserved(Data(instructions.front()));
            instructions.pop();
        }

        setDefinition(name, Definition(name, entry));
    }

    /*
     * Get the entry point for a symbol.
     */
    unsigned long
    definitionEntry (std::string name)
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
        stack.reservedRollback(idx);
    }

    /*
     * Execute starting from the given index on the stack, starting from the
     * bottom and moving up.
     */
    void
    execute (unsigned long entry_point)
    {
        PC = entry_point;
        registers[REGBASE] = stack.index();

        while (true) {
            Data *data = stack.reserved(PC);
            PC++;

            if (!data->isExecutable || data->type != DATA_CODE)
                fatal("Cannot execute non-executable data at index %d", PC);

            Bytecode bc = data->bytecode();
            switch (bc.op) {
                case OP_HALT:   return;
                case OP_MOVESTR:  move(bc.data.integer(), bc.reg1); break;
                case OP_MOVE:   move(bc.data.integer(), bc.reg1); break;
                case OP_PUSH:   push(bc.reg1); break;
                case OP_POP:    pop(bc.reg1); break;
                case OP_PRINT:  print((long) bc.data.integer()); break;
                case OP_ADD:    add(); break;
                case OP_CALL:   call(); break; 
                case OP_RET:    ret(); break;
                case OP_DEFINE: define(); break;

                case OP_NULL:
                    fatal("NULL bytecode operator!");
                default:
                    fatal("unimplemented bytecode operator!");
            }
        }
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
        printf("0x%08lx\n", data.integer());
    }

    void
    call ()
    {
        Data base = registers[REGBASE];
        Data target = registers[REGCALL];
        stack.push(Data(PC));
        stack.push(Data(base.integer()));
        PC = target.integer();
        registers[REGBASE] = stack.index();
    }

    void
    ret ()
    {
        unsigned long floor = registers[REGBASE].integer();
        while (stack.index() > floor)
            stack.pop();
        Data base = stack.pop();
        Data addr = stack.pop();
        PC = addr.integer();
        registers[REGBASE] = Data(base.integer());
    }

    void
    add ()
    {
        Data d1 = stack.pop();
        Data d2 = stack.pop();
        stack.push(Data(d1.integer() + d2.integer()));
    }

    /* define the instructions on the stack as a symbol to be used later */
    void
    define ()
    {
        auto var1 = reg(REG1);
        auto var2 = reg(REG2);
        assert(var1.type == DATA_STR);
        assert(var2.type == DATA_INTEGER);
        auto name = var1.string();
        auto entry = var2.integer();

        setDefinition(name, Definition(name, entry));
    }

protected:
    Stack stack;
    std::vector<Data> registers;
    unsigned long PC; /* program counter */

    std::map<std::string, Definition> _definitions;

    void
    setDefinition (std::string name, Definition def)
    {
        _definitions[name] = def;
        printf("defined `%s' at %lu\n", name.c_str(), def.index);
    }
};

#endif
