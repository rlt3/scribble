#ifndef SCRIBBLE_COMPILE
#define SCRIBBLE_COMPILE

#include <vector>
#include "token.hpp"
#include "bytecode.hpp"

class Compile
{
public:
    Compile ()
    {
    }

    Bytecode
    token (std::vector<Token> tokens)
    {
        return Bytecode();
    }
};

#endif
