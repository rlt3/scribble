#ifndef SCRIBBLE_COMPILE
#define SCRIBBLE_COMPILE

#include <vector>
#include <cassert>
#include "token.hpp"
#include "bytecode.hpp"
#include "machine.hpp"

struct Definition
{
    std::string name;
    unsigned long index;
    unsigned long length;

    Definition()
        : name("NULL")
        , index(0)
        , length(0)
    {}

    Definition(std::string name, unsigned long index, unsigned long length)
        : name(name)
        , index(index)
        , length(length)
    {}
};

class Compile
{
public:
    Compile (Machine& machine)
        : _machine(machine)
    {
        define("foobar", std::queue<Bytecode>({
            Bytecode(OP_MOVE, REG1, 1000),
            Bytecode(OP_MOVE, REG2,  337),
            Bytecode(OP_PUSH, REG1),
            Bytecode(OP_PUSH, REG2),
            Bytecode(OP_ADD),
            Bytecode(OP_POP, REG1),
            Bytecode(OP_RET)
        }));

        define("add", std::queue<Bytecode>({
            Bytecode(OP_PUSH, REG1),
            Bytecode(OP_PUSH, REG2),
            Bytecode(OP_ADD),
            Bytecode(OP_POP, REG1),
            Bytecode(OP_RET)
        }));
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
    std::map<std::string, Definition> _definitions;

    Definition&
    findDefinition (std::string name)
    {
        return _definitions[name];
    }

    void
    define (std::string name, std::queue<Bytecode> bc)
    {
        auto def = Definition(name, _machine.writeReserved(bc), bc.size());
        _definitions[name] = def;
        printf("defined `%s' at %lu\n", name.c_str(), def.index);
    }

    /* primitive = <string> | <integer> */
    void
    primitive ()
    {
        if (peek().type != TKN_INTEGER)
            fatal("Unimplemented primitive value!");

        Token tkn = next();
        _bytecode.push(Bytecode(OP_MOVE, REG1, tkn.toUInt()));
        _bytecode.push(Bytecode(OP_PUSH, REG1));
    }

    /* call = <symbol> | <symbol>([ <expr> ]*) */
    void
    call ()
    {
        Token tkn = next();
        assert(tkn.type == TKN_SYMBOL);

        auto iter = _definitions.find(tkn.str);
        if (iter == _definitions.end())
            fatal("Cannot call undefined symbol `%s'", tkn.str.c_str());

        Definition& def = iter->second;
        _bytecode.push(Bytecode(OP_CALL, def.index));
    }

    /* expr = <call> | <primitive> */
    void
    expr ()
    {
        auto def = findDefinition("foobar");
        _bytecode.push(Bytecode(OP_MOVE, REG1, def.index));
        _bytecode.push(Bytecode(OP_CALL));
        _bytecode.push(Bytecode(OP_PUSH, REG1));
        _bytecode.push(Bytecode(OP_PRINT));
        _bytecode.push(Bytecode(OP_HALT));
    }
};

#endif
