#ifndef SCRIBBLE_COMPILE
#define SCRIBBLE_COMPILE

#include <vector>
#include <cassert>
#include "token.hpp"
#include "bytecode.hpp"
#include "machine.hpp"
#include "frame.hpp"

typedef enum {
    RSRV_NULL = 0,
    RSRV_DEFINE,
} ReservedSymbol;

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
            fatal("Expected token of type `%s` but received `%s`!",
                    tokenTypeString(type), tokenTypeString(t.type));
        return t;
    }

    bool
    eof ()
    {
        return _tokens.empty();
    }

private:
    /* literal = <string> | <integer> | <symbol> */
    void
    literal (std::queue<Bytecode> &bc, Token &token)
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
                fatal("Non-literal token encountered: `%s`!",
                        token.str.c_str());
        }
        bc.push(Bytecode(op, reg, token.toPrimitive()));
        bc.push(Bytecode(OP_PUSH, reg));
    }

    /*
     * <define> := <symbol>(<symbol> ([<symbol>]*) [<expr>]*)
     *
     * Compile to a custom, local set of Bytecode and then define it as a
     * procedure. Push the name of that procedure as the return value.
     */
    void
    define (std::queue<Bytecode> &bc)
    {
        Token name;
        std::vector<Token> args;
        std::queue<Bytecode> body;

        expect(TKN_LPAREN);
        name = expect(TKN_SYMBOL);

        expect(TKN_LPAREN);
        while (peek().type != TKN_RPAREN)
            args.push_back(expect(TKN_SYMBOL));
        expect(TKN_RPAREN);

        while (peek().type != TKN_RPAREN)
            expr(body);
        expect(TKN_RPAREN);

        body.push(Bytecode(OP_RET));
        _machine.defineProcedure(name.str.c_str(), args.size(), body);

        literal(bc, name);
    }

    void
    reserved (std::queue<Bytecode> &bc, ReservedSymbol &symbol)
    {
        switch (symbol) {
            case RSRV_DEFINE: define(bc); break;
            default:
                fatal("Unimplemented or erroneous ReservedSymbol");
        }
    }

    bool
    isReserved (Token &token, ReservedSymbol &symbol)
    {
        if (token.str == "define") {
            symbol = RSRV_DEFINE;
            return true;
        }
        symbol = RSRV_NULL;
        return false;
    }

    /* <list> := ([<expr> ]*) */
    void
    list (std::queue<Bytecode> &bc)
    {
        assert(0);
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

    /* <expr> := <reserved> | <call> | <list> | <literal> */
    void
    expr (std::queue<Bytecode> &bc)
    {
        Token token = next();
        if (token.type == TKN_SYMBOL) {
            ReservedSymbol reserved_symbol;
            if (isReserved(token, reserved_symbol)) {
                reserved(bc, reserved_symbol);
                return;
            }

            if (peek().type == TKN_LPAREN) {
                call(bc, token);
                return;
            }
        }
        else if (token.type == TKN_LPAREN) {
            list(bc);
            return;
        }

        literal(bc, token);
    }
};

/*
 * <integer> := [0-9]+
 * <string> := "[A-Za-z0-9 ]*"
 * <symbol> := [A-Za-z]+[0-9]*
 * <literal> := <string> | <integer>
 * <reserved> := push | pop | define | print | add ; etc.
 * <call> := <symbol>([<expr> ]*)
 * <list> := ([<expr> ]*)
 * <expr> := <reserved> | <symbol> | <call> | <list> | <literal>
 *
 * Ancestors are simply special pre-defined procedures which make direct use of
 * machine primitives. All newly defined procedures may use
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
