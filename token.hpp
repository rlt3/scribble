#ifndef SCRIBBLE_TOKEN
#define SCRIBBLE_TOKEN

#include "error.hpp"
#include "primitive.hpp"

typedef enum {
    TKN_INVALID,
    TKN_STRING,
    TKN_INTEGER,
    TKN_FLOAT,
    TKN_SYMBOL,
    TKN_LPAREN,
    TKN_RPAREN,
    TKN_EOF,
} TokenType;

struct Token
{
    TokenType type;
    std::string str;

    Token ()
        : type(TKN_INVALID)
        , str("")
    {
    }

    Token (TokenType type)
        : type(type)
        , str("")
    {
    }

    Token (TokenType type, std::string str)
        : type(type)
        , str(str)
    {
    }

    Primitive
    toPrimitive ()
    {
        switch (type) {
            case TKN_STRING:  return Primitive(str);
            case TKN_INTEGER: return Primitive(std::stoul(str));
            default:
                fatal("Unimplemented token -> primitive conversion! `%d'", type);
        }
    }

    std::string
    typestring ()
    {
        switch (type) {
            case TKN_STRING:  return "String";
            case TKN_INTEGER: return "Integer";
            case TKN_FLOAT:   return "Float";
            case TKN_SYMBOL:  return "Symbol";
            case TKN_LPAREN:  return "(";
            case TKN_RPAREN:  return ")";
            case TKN_INVALID:
            default:          return "!!BAD!!";
        }
    }
};

#endif
