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
    tokens (std::queue<Token> t)
    {
        _tokens = t;
        return Bytecode();
    }

protected:
    std::queue<Token> _tokens;

    Token
    peek ()
    {
        if (_tokens.empty())
            return Token(TKN_EOF);
        return _tokens.front();
    }

    Token
    next ()
    {
        Token t = peek();
        if (t.type != TKN_EOF)
            _tokens.pop();
        return t;
    }

    bool
    eof ()
    {
        return _tokens.empty();
    }

private:
};

#endif
