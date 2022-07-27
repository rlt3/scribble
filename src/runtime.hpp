#ifndef SCRIBBLE_JIT
#define SCRIBBLE_JIT

#include <string>
#include <stack>

#include "llvm.hpp"
#include "ir.hpp"
#include "procedure.hpp"
#include "primitive.hpp"

static std::stack<PrimitiveType> _typestack;

extern "C" {
    void
    typestack_pushInteger ()
    {
        _typestack.push(PRM_INTEGER);
    }

    void
    typestack_pushString ()
    {
        _typestack.push(PRM_STRING);
    }
}

class Runtime
{
protected:
    LLVM llvm;
    IR globals;
    IR externals;

public:
    Runtime ()
        /*
         * Define the values which should exist in all LLVM modules, e.g. the
         * stack, and the external declarations that are needed to access them
         * in each module.
         */
        : globals(IR(
            "@stack = global [4096 x i64] zeroinitializer, align 16\n"
            "@top = global i64* getelementptr inbounds ([4096 x i64], [4096 x i64]* @stack, i32 0, i32 0), align 8\n"
        ))
        , externals(IR(
            "@stack = external global [4096 x i64]\n"
            "@top = external global i64*\n"
            "declare void @typestack_pushInteger ()\n"
            "declare void @typestack_pushString ()\n"
        ))

    {
        /*
         * Initialize global state in the JIT.
         */
        defineIR(globals);
    }

    void
    defineIR (IR ir)
    {
        llvm.defineIR(ir.getString());
    }

    void
    executeProcedure (Procedure &p)
    {
        llvm.execute(p.getName(), externals.getString() + p.getIRString());
    }

    unsigned long*
    getStack ()
    {
        return llvm.getStack();
    }

    std::stack<PrimitiveType>&
    getTypestack ()
    {
        return _typestack;
    }
};

#endif
