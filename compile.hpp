#ifndef SCRIBBLE_COMPILE
#define SCRIBBLE_COMPILE

#include <vector>
#include <cassert>
#include "token.hpp"
#include "bytecode.hpp"
#include "machine.hpp"
#include "frame.hpp"

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

    void
    integer ()
    {
        assert(peek().type == TKN_INTEGER);
        Token tkn = next();
        _bytecode.push(Bytecode(OP_MOVESTR, REG1, tkn.toPrimitive()));
        _bytecode.push(Bytecode(OP_PUSH, REG1));
    }

    void
    string ()
    {
        assert(peek().type == TKN_STRING);
        Token tkn = next();
        _bytecode.push(Bytecode(OP_MOVE, REG1, tkn.toPrimitive()));
        _bytecode.push(Bytecode(OP_PUSH, REG1));
    }

    /* primitive = <string> | <integer> */
    void
    primitive ()
    {
        if (peek().type == TKN_STRING)
            return string();
        else
            return integer();
    }

    /* <symbol> := [A-Za-z]+[0-9]* */
    void
    symbol (Token &symbol)
    {
    }

    /* <call> := <symbol>([<expr> ]*) */
    void
    call (Token &symbol)
    {
    }

    void
    ancestor ()
    {
        /*
         * Where we will handle 'primitives' of the runtime like defining new
         * symbols or things like simple addition which do not need to be
         * separate functions but can be 'inlined'.
         */
    }

    bool
    isAncestor (Token &t)
    {
        return false;
    }

    /* <list> := ([<expr> ]*) */
    void
    list ()
    {
    }

    /* <expr> := <ancestor> | <symbol> | <call> | <list> | <primitive> */
    void
    expr ()
    {
        Token token = peek();
        if (token.type == TKN_SYMBOL) {
            if (isAncestor(token)) {
                ancestor();
                return;
            }

            next();
            if (peek().type == TKN_LPAREN) {
                call(token);
                return;
            }

            symbol(token);
            return;
        }

        if (token.type == TKN_LPAREN) {
            list();
            return;
        }

        primitive();
    }
};

/*
 * <integer> := [0-9]+
 * <string> := "[A-Za-z0-9 ]*"
 * <symbol> := [A-Za-z]+[0-9]*
 * <primitive> := <string> | <integer>
 * <ancestor> := push | pop | define | print | add ; etc.
 * <call> := <symbol>([<expr> ]*)
 * <list> := ([<expr> ]*)
 * <expr> := <ancestor> | <symbol> | <call> | <list> | <primitive>
 *
 * Ancestors are simply special symbols. Instead of calling, we consider these
 * apart of the machine and call them directly, thus their special handling.
 * All other symbols can simply be a call. Thus all procedures of the machine
 * are made of ancestors and their descendants.
 *

define(leroy ()
    push("leroy")
    print())
=>
define leroy
movestr "leroy" reg1
push reg1
print
ret
halt

define(double (x)
    push(x)
    push(x)
    add())
=>
define double
push reg1
push reg1
add
ret
halt

define(leet ()
    push(double(500))
    push(337)
    print())
=>
define leet
move 500 reg1
call index(double)
push reg1
mov 337 reg1
push reg1
print
ret
halt

*/

#endif
