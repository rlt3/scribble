#ifndef SCRIBBLE_TOKEN
#define SCRIBBLE_TOKEN

typedef enum {
    TKN_INVALID,
    TKN_STRING,
    TKN_INTEGER,
    TKN_FLOAT,
    TKN_NAME,
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

    Token (TokenType type, std::string str)
        : type(type)
        , str(str)
    {
    }
};

#endif
