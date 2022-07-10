#ifndef SCRIBBLE_IR
#define SCRIBBLE_IR

#include <string>
#include <vector>

class IR {
public:
    IR (std::string ir)
        : _finalstr(ir)
    {}

    std::string
    getString ()
    {
        return _finalstr;
    }

protected:
    std::string _finalstr;
};

#endif
