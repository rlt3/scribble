#ifndef SCRIBBLE_PROCEDURE
#define SCRIBBLE_PROCEDURE

#include <string>
#include <vector>
#include "ir.hpp"

/*
 * The procedure owns information about the given IR. This information includes
 * the callees and callers of this procedure which may be used by the JIT. The
 * procedure owns its IR representation and (TODO) allows for insertions of
 * instrumentation.
 */
class Procedure
{
public:
    Procedure (IR ir, unsigned num_args);

    /* Add a procedure which calls this procedure */
    void
    addCaller (std::string name);

    /* Add a procedure which this procedure calls */
    void
    addCallee (std::string name);

    /* 
     * Add an instrumentation function either before the procedure is called,
     * after it has been called.
     */
    void
    addInstrumentBefore ();
    void
    addInstrumentAfter ();

protected:
    unsigned num_args;
    std::vector<std::string> caller;
    std::vector<std::string> callee;
};

#endif
