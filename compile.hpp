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
    {}

    std::queue<Bytecode>
    tokens (std::queue<Token> tokens)
    {
        std::queue<Bytecode> bc;
        _tokens = tokens;
        expr(bc);
        bc.push(Bytecode(OP_HALT));
        return bc;
    }

protected:
    Machine& _machine;
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
        assert(0);
    }

    /* primitive = <string> | <integer> | <symbol> */
    void
    primitive (std::queue<Bytecode> &bc, Token &token /*, int arg*/)
    {
        Register reg = REG1;
        Operator op;
        switch (token.type) {
            case TKN_STRING:
                assert(token.type == TKN_STRING);
                op = OP_MOVESTR;
                break;

            case TKN_INTEGER:
                assert(token.type == TKN_INTEGER);
                op = OP_MOVEINT;
                break;

            case TKN_SYMBOL:
                assert(token.type == TKN_SYMBOL);
                op = OP_MOVESYM;
                break;

            default:
                fatal("Non-primitive token encountered: `%s`!", token.str.c_str());
        }
        bc.push(Bytecode(op, reg, token.toPrimitive()));
        bc.push(Bytecode(OP_PUSH, reg));
    }

    void
    reserved (std::queue<Bytecode> &bc, Token &symbol)
    {
        /*
         * Where we will handle runtime-only reserved procedures such as
         * `define'.
         */
        assert(0);
    }

    bool
    isReserved (Token &token)
    {
        //if (token.str == "define")
        //    return true;
        return false;
    }

    /* <list> := ([<expr> ]*) */
    void
    list (std::queue<Bytecode> &bc)
    {
        assert(0);

        /*
         * Trying to determine how to handle lists. I'm thinking in terms of
         * defining the `define` procedure. I really want to design those
         * primitives using what is already available.
         *
         * What the machine requires for calls is a discrete list of arguments.
         * I've decided to go with registers 1-4 as argument registers, i.e. a
         * max of 4 arguments to any call. How do I parse the list after the
         * name of the procedure into discrete elements, i.e to somehow know
         * when to evaluate into one register or another?
         *
         * If we can simply parse the list to its bytecode equivalent, we can
         * evaluate that list and walk the stack for a max of four values. We
         * are using the machine itself to parse its own input.
         */
    }

    /* <call> := <symbol>([<expr> ]*) */
    void
    call (std::queue<Bytecode> &bc, Token &symbol)
    {
        assert(symbol.type == TKN_SYMBOL);

        /* parse expressions first to allow arguments onto stack for call */
        expect(TKN_LPAREN);
        while (peek().type != TKN_RPAREN)
            expr(bc);
        next();

        bc.push(Bytecode(OP_CALL, Primitive(PRM_SYMBOL, symbol.str)));
    }

    /* <expr> := <reserved> | <call> | <list> | <primitive> */
    void
    expr (std::queue<Bytecode> &bc)
    {
        Token token = next();
        if (token.type == TKN_SYMBOL) {
            if (isReserved(token)) {
                reserved(bc, token);
                return;
            }

            if (peek().type == TKN_LPAREN) {
                call(bc, token);
                return;
            }

            goto prim;
        }

        if (token.type == TKN_LPAREN) {
            list(bc);
            return;
        }

prim:
        primitive(bc, token);
    }
};

/*
 * <integer> := [0-9]+
 * <string> := "[A-Za-z0-9 ]*"
 * <symbol> := [A-Za-z]+[0-9]*
 * <primitive> := <string> | <integer>
 * <reserved> := push | pop | define | print | add ; etc.
 * <call> := <symbol>([<expr> ]*)
 * <list> := ([<expr> ]*)
 * <expr> := <reserved> | <symbol> | <call> | <list> | <primitive>
 *
 * Ancestors are simply special pre-defined procedures which make direct use of
 * machine primitives.  the machine. All newly defined procedures may use
 * ancestor procedures. The only procedures able to be defined at the beginning
 * will be ancestor procedures as they are the building blocks of any program.
 * Thus all procedures of the machine are made of ancestors and their
 * descendants.
 *
 * Some examples include "print", "add", "div" and will probably have stack
 * analogues too -- "swap", "pop", "push", etc.
 *

define(leroy ()
    "leroy"
    print())
=>
define leroy
loadstr -2 reg1 ; "leroy" interned at address -2
push reg1
print
ret
halt

define(double (x)
    add(x x))
=>
define double
load 0, reg1
push reg1
push reg1
add
ret
halt

define(add-then-double (x y)
    double(add(x y)))
=>
define add-then-double
load 0, reg1
load 1, reg2
push reg1
push reg2
add
double
ret
halt

define(leet ()
    add(double(500) 337)
    print)
=>
define leet
move 500 reg1
push reg1
call double
mov 337 reg1
push reg1
add
print
ret
halt

*/

#endif
