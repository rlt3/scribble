#ifndef SCRIBBLE_PARSE

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cctype>

#include "definitions.hpp"

/*
 * Language:
 *
 * string: "[A-Za-z0-9 ]+" ; any string wrapped in quotes
 * number: [0-9]+ ; integer numbers only
 * name: [A-Za-z0-9]+ ; any string without a space
 * symbol: <string> | <number> | <name>
 * word: <symbol>[(<word>[ <word>]*)]
 * sentence: <word>[ <word>]*
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

    std::vector<std::string>
    tokenize ()
    {
        sentence();
        return tokens;
    }

protected:
    std::vector<std::string> tokens;
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
        printf("`%c`(%d) != `%c`(%d)\n", c, c, e, e);
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
    std::string&
    string ()
    {
        int c;
        std::string str;

        expect('"');
        while (true) {
            c = next();
            if (eof())
                fatal("Expected end of string `\"', got instead: %c\n", c);
            if (c == '"')
                break;
            str += c;
        }

        tokens.push_back(str);
        return tokens.back();
    }

    void
    number ()
    {
    }

    std::string&
    name ()
    {
        int c;
        std::string str;

        while (true) {
            c = peek();
            if (eof() || isspace(c) || isparen(c))
                break;
            str += next();
        }

        tokens.push_back(str);
        return tokens.back();
    }

    std::string&
    symbol ()
    {
        if (peek() == '"')
            return string();
        else
            return name();
    }

    std::string
    word ()
    {
        std::string self = symbol();
        self += "( ";
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
                self += word();
                self += " ";
            }
        }

        self += ")";
        return self;
    }

    void
    sentence ()
    {
        while (!eof()) {
            skipwhitespace();
            printf("| %s\n", word().c_str());
        }
    }
};

#endif
