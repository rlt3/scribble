#ifndef SCRIBBLE_JIT
#define SCRIBBLE_JIT

class Procedure;

/*
 * The JIT handles the execution of the IR that is generated. It also owns all
 * the internal references of native procedures and the control-flow graph of
 * them.
 */
class JIT
{
public:
    JIT ()
    {
    }

    /*
     * Create a function definition in the JIT using the given string as a name
     * which executes the given procedure when called. If that definition
     * already exists, then update it instead.
     */
    void
    createOrUpdateProcedure (std::string name, Procedure proc);

    /*
     * The entry point in the JIT isn't a hard address, but the internal name
     * for the latest version of the procedure, e.g. the high-level name of a
     * procedure may be `foobar', but after three revisions, the current
     * internal name may be `foobar_3_void_int`.
     */
    std::string&
    getProcedureEntry (std::string name);

    /*
     * Execute the given procedure. This procedure is not meant to be defined
     * for future use.
     */
    void
    executeProcedure (Procedure proc);


protected:
};

#endif
