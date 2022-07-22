#ifndef SCRIBBLE_PROCEDURE
#define SCRIBBLE_PROCEDURE

#include <string>
#include <vector>
#include "ir.hpp"

/*
 * The procedure owns information about the given IR. This information includes
 * the callees and callers of this procedure which may be used by the JIT,
 * arguments, and the name. The procedure owns its IR representation and (TODO)
 * allows for insertions of instrumentation.
 */
class Procedure
{
public:
    Procedure (std::string name, unsigned num_args, IR ir)
        : name(name)
        , num_args(num_args)
        , ir(ir)
    {}

    std::string
    getName ()
    {
        return name;
    }

    std::string
    getIRString ()
    {
        return ir.getString();
    }

    /* Add a procedure which calls this procedure */
    void
    addCaller (std::string name)
    {
        callers.push_back(name);
    }

    /* Add a procedure which this procedure calls */
    void
    addCallee (std::string name)
    {
        callees.push_back(name);
    }

    /* 
     * Add an instrumentation function either before the procedure is called,
     * after it has been called.
     */
    void
    addInstrumentBefore ();
    void
    addInstrumentAfter ();

protected:
    std::string name;
    unsigned num_args;
    IR ir;
    std::vector<std::string> callers;
    std::vector<std::string> callees;
};

#endif
