#ifndef SCRIBBLE_PARSE
#define SCRIBBLE_PARSE

#include <queue>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <cctype>

#include "definitions.hpp"
#include "token.hpp"
#include "expression.hpp"

/*
 * Language:
 *
 * string: "[A-Za-z0-9 ]+" ; any string wrapped in quotes
 * number: [0-9]+ ; integer numbers only
 * name: [A-Za-z0-9]+ ; any string without a space
 * expression: <name>[(<expression>*)] | <string> | <number>
 * list: <expression>*
 *
 * Every parenthesis is a new list. Lists are just linear collections of
 * symbols which should exist in the program. Lists can be named and anonymous.
 * Anonymous lists are much like anonymous lambda functions, good for glueing
 * together pieces of code. Naming a list is like creating a regular function.
 *
 * Imagine having an if-statement:
 *      if(true(value)
 *          expr1
 *          expr2
 *          expr3
 *          ...)
 *
 * It has the same problem of any Lisp, the syntax is tiny therefore the only
 * solution is to introduce more grouping. As an example, here's an if-else
 * statement:
 *      if-else(true(value)
 *          (a b c d)
 *          (e f g h)
 *      )
 * The idea here being that the `if-else` symbol has been defined as having
 * three arguments: one truth, a group of expressions to run if true, and
 * one to run if false. These last two groups are anonymous lists.
 */

class Parse
{
public:
    Parse (std::istream& input)
        : _stream(input)
    { }

    /* Parse the input stream for a single expression */
    std::queue<Token>
    stream ()
    {
        _tokens = std::queue<Token>();

        expr();

        _tokens.push(TKN_EOF);
        return _tokens;
    }

protected:
    std::queue<Token> _tokens;
    std::istream& _stream;

    int
    next ()
    {
        return _stream.get();
    }

    int
    peek ()
    {
        if (eof())
            return EOF;
        return _stream.peek();
    }

    void
    expect (int e)
    {
        int c = next();
        if (c != e)
            fatal("Expected `%c` but received `%c`", e, c);
    }

    /* Skips any available whitespace but stops at new lines */
    void
    skipwhitespace ()
    {
        while (!eof() && isspace(peek()))
            next();
    }

    bool
    eof ()
    {
        return _stream.eof();
    }

    bool
    isparen (int c)
    {
        return c == ')' || c == '(';
    }

private:
    void
    string ()
    {
        int c;
        std::string str;

        expect('"');
        while (true) {
            c = next();
            if (eof())
                fatal("Encountered end-of-file before terminating string");
            if (c == '"')
                break;
            str += c;
        }

        _tokens.push(Token(TKN_STRING, str));
    }

    void
    number ()
    {
        int c;
        std::string str;

        while (true) {
            c = peek();
            if (eof() || isspace(c) || isparen(c))
                break;
            if (!isdigit(c))
                fatal("Expected digit, got `%c' instead", c);
            str += next();
        }

        _tokens.push(Token(TKN_INTEGER, str));
    }

    void
    name ()
    {
        int c;
        std::string str;

        c = peek();
        if (!isalpha(c))
            fatal("Names cannot begin with numbers or digits: '%c'", c);

        while (true) {
            c = peek();
            if (eof() || isspace(c) || isparen(c))
                break;
            str += next();
        }

        _tokens.push(Token(TKN_SYMBOL, str));
    }

    /* list: ([<expr>]*) */
    void
    list ()
    {
        expect('(');
        _tokens.push(Token(TKN_LPAREN));
        while (!eof() && peek() != ')')
            expr();
        expect(')');
        _tokens.push(Token(TKN_RPAREN));
    }

    /* expr: <string> | <number> | <list> | <name>([<list>]*) */
    void
    expr ()
    {
        skipwhitespace();
        if (peek() == '"') {
            string();
        }
        else if (isdigit(peek())) {
            number();
        }
        else if (peek() == '(') {
            list();
        }
        else {
            name();
            if (peek() == '(')
                list();
        }
    }
};

#endif
