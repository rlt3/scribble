#ifndef SCRIBBLE_COMPILE
#define SCRIBBLE_COMPILE

#include <vector>
#include <cassert>
#include "token.hpp"
#include "bytecode.hpp"
#include "machine.hpp"

class Compile
{
public:
    Compile (Machine& machine)
        : _machine(machine)
    {
        //define("foobar", std::queue<Bytecode>({
        //    Bytecode(OP_MOVE, REG1, 1000),
        //    Bytecode(OP_MOVE, REG2,  337),
        //    Bytecode(OP_PUSH, REG1),
        //    Bytecode(OP_PUSH, REG2),
        //    Bytecode(OP_ADD),
        //    Bytecode(OP_POP, REG1),
        //    Bytecode(OP_RET)
        //}));

        //define("add", std::queue<Bytecode>({
        //    Bytecode(OP_PUSH, REG1),
        //    Bytecode(OP_PUSH, REG2),
        //    Bytecode(OP_ADD),
        //    Bytecode(OP_POP, REG1),
        //    Bytecode(OP_RET)
        //}));
    }

    std::queue<Bytecode>
    tokens (std::queue<Token> t)
    {
        _tokens = t;
        expr();
        return _bytecode;
    }

protected:
    Machine& _machine;
    std::queue<Token> _tokens;
    std::queue<Bytecode> _bytecode;

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
    void
    define (std::string name, std::queue<Bytecode> bc)
    {
        _machine.defineBytecode(name, bc);
    }

    /* primitive = <string> | <integer> */
    void
    primitive ()
    {
        if (peek().type != TKN_INTEGER)
            fatal("Unimplemented primitive value!");

        Token tkn = next();
        _bytecode.push(Bytecode(OP_MOVE, REG1, tkn.toPrimitive()));
        _bytecode.push(Bytecode(OP_PUSH, REG1));
    }

    /* call = <symbol> | <symbol>([ <expr> ]*) */
    void
    call ()
    {
        Token tkn = next();
        assert(tkn.type == TKN_SYMBOL);
        _bytecode.push(Bytecode(OP_CALL, _machine.definitionEntry(tkn.str)));
    }

    /* expr = <call> | <primitive> */
    void
    expr ()
    {
        auto entry = _machine.definitionEntry("foobar");
        _bytecode.push(Bytecode(OP_MOVE, REG1, entry));
        _bytecode.push(Bytecode(OP_CALL));
        _bytecode.push(Bytecode(OP_PUSH, REG1));
        _bytecode.push(Bytecode(OP_PRINT));
        _bytecode.push(Bytecode(OP_HALT));
    }
};

#endif
