#ifndef SCRIBBLE_EXPRESSION
#define SCRIBBLE_EXPRESSION

#include <vector>
#include <cstdio>
#include "token.hpp"

typedef enum {
    EXPR_NULL,
    EXPR_PRIMITIVE,
    EXPR_SYMBOL,
    EXPR_LIST
} ExprType;

class Expression
{
public:
    Expression (Token& token)
        : token(token)
    {
    }

    void
    addChild (Expression expr)
    {
        _children.push_back(expr);
    }

    const std::vector<Expression>&
    children ()
    {
        return _children;
    }

    ExprType
    type ()
    {
        switch (token.type) {
            case TKN_STRING:
            case TKN_INTEGER:
            case TKN_FLOAT:
                return EXPR_PRIMITIVE;

            case TKN_NAME:
                return EXPR_SYMBOL;

            case TKN_INVALID:
            default:
                fatal("Got invalid token in expression!");
        }
        return EXPR_NULL;
    }

    unsigned long
    integer ()
    {
        return std::stoi(token.str);
    }

    std::string
    string ()
    {
        return token.str;
    }

    void
    print ()
    {
        print(0);
    }

    void
    print (int idx)
    {
        putchar('|');
        for (int i = 0; i < idx; i++)
            putchar(' ');

        printf("%s", token.str.c_str());
        if (token.type != TKN_NAME) {
            putchar('\n');
            return;
        }
        printf("(\n");

        for (auto child : _children)
            child.print(idx + 4);

        putchar('|');
        for (int i = 0; i < idx; i++)
            putchar(' ');
        printf(")\n");
    }

protected:
    Token& token;
    std::vector<Expression> _children;
};

#endif
