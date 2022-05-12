#ifndef SCRIBBLE_PARSE
#define SCRIBBLE_PARSE

#include <string>
#include <vector>
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
    Parse (std::istream& stream)
        : stream(stream)
    {
    }

    void
    tokenize ()
    {
        list();
    }

    Token&
    intern (TokenType type, std::string str)
    {
        auto iter = tokens.find(str);
        if (iter == tokens.end()) {
            tokens[str] = Token(type, str);
        }
        return tokens[str];
    }

protected:
    std::map<std::string, Token> tokens;
    std::istream& stream;

    int
    next ()
    {
        return stream.get();
    }

    int
    peek ()
    {
        return stream.peek();
    }

    void
    expect (int e)
    {
        int c = next();
        if (c != e)
            fatal("Expected `%c` but received `%c`", e, c);
    }

    void
    skipwhitespace ()
    {
        while (!eof() && isspace(peek()))
            next();
    }

    bool
    eof ()
    {
        return stream.eof();
    }

    bool
    isparen (int c)
    {
        return c == ')' || c == '(';
    }

private:
    Token&
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

        return intern(TKN_STRING, str);
    }

    Token&
    number ()
    {
        int c;
        std::string str;

        while (true) {
            c = peek();
            if (eof() || isspace(c))
                break;
            if (!isdigit(c))
                fatal("Expected digit, got `%c' instead", c);
            str += next();
        }

        return intern(TKN_INTEGER, str);
    }

    Token&
    name ()
    {
        int c;
        std::string str;

        c = peek();
        if (!isalpha(c))
            fatal("Names cannot begin with numbers or digits");

        while (true) {
            c = peek();
            if (eof() || isspace(c) || isparen(c))
                break;
            str += next();
        }

        return intern(TKN_NAME, str);
    }

    Expression
    expression ()
    {
        if (peek() == '"')
            return Expression(string());
        else if (isdigit(peek()))
            return Expression(number());

        Expression expr(name());
        skipwhitespace();

        if (peek() == '(') {
            next();
            while (true) {
                skipwhitespace();
                if (eof())
                    expect(')');
                if (peek() == ')') {
                    next();
                    break;
                }
                expr.addChild(expression());
            }
        }

        return expr;
    }

    void
    list ()
    {
        while (!eof()) {
            skipwhitespace();
            auto expr = expression();
            expr.print();
        }
    }
};

#endif
