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

    Token
    expect (TokenType type)
    {
        Token t = next();
        if (t.type != type)
            fatal("Unexpected token type!");
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

    /* call = <symbol>([ <expr> ]*) */
    void
    call ()
    {
        Token tkn = expect(TKN_SYMBOL);
        expect(TKN_LPAREN);

        //_bytecode.push(Bytecode(OP_CALL, _machine.definitionEntry(tkn.str)));
    }

    /* expr = <symbol> | <call> | <primitive> */
    void
    expr ()
    {
        auto entry = Primitive("leet");
        _bytecode.push(Bytecode(OP_CALL, entry));
        _bytecode.push(Bytecode(OP_HALT));
    }
};

#endif
