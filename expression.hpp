#ifndef SCRIBBLE_EXPRESSION
#define SCRIBBLE_EXPRESSION

#include <vector>
#include <cstdio>
#include "token.hpp"

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
        children.push_back(expr);
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

        for (auto child : children)
            child.print(idx + 4);

        putchar('|');
        for (int i = 0; i < idx; i++)
            putchar(' ');
        printf(")\n");
    }

protected:
    Token& token;
    std::vector<Expression> children;
};

#endif
