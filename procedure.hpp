#ifndef SCRIBBLE_PROCEDURE
#define SCRIBBLE_PROCEDURE

#include <string>

struct Procedure
{
    std::string name;
    unsigned long entry;
    unsigned long nargs;
    bool is_compiled;

    Procedure()
        : name("NULL")
        , entry(0)
        , nargs(0)
        , is_compiled(false)
    {}

    Procedure(std::string name, unsigned long entry, unsigned long nargs)
        : name(name)
        , entry(entry)
        , nargs(nargs)
        , is_compiled(false)
    {}
};

#endif
