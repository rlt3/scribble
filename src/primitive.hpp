#ifndef SCRIBBLE_PRIMITIVE
#define SCRIBBLE_PRIMITIVE

#include <cassert>
#include <string>

typedef enum {
    PRM_NULL,
    PRM_STRING,
    PRM_INTEGER,
    PRM_SYMBOL,
    NUM_PRM
} PrimitiveType;

struct Primitive
{
    Primitive () : _type(PRM_NULL) {}
    Primitive (std::string s) : _type(PRM_STRING), _string(s) {}
    Primitive (unsigned long v) : _type(PRM_INTEGER), _integer(v) {}
    Primitive (PrimitiveType type, std::string s) : _type(type), _string(s) {}

    PrimitiveType
    type ()
    {
        return _type;
    }

    std::string
    symbol ()
    {
        assert(_type == PRM_SYMBOL);
        return _string;
    }

    std::string
    string ()
    {
        assert(_type == PRM_STRING);
        return _string;
    }

    unsigned long
    integer ()
    {
        assert(_type == PRM_INTEGER);
        return _integer;
    }

    std::string
    toString ()
    {
        switch (_type) {
            case PRM_SYMBOL:
                return _string;
                break;

            case PRM_STRING:
                return "\"" + _string + "\"";
                break;

            case PRM_INTEGER:
                return std::to_string(_integer);
                break;

            default:
                return "NULL";
                break;
        }
    }

    void
    print ()
    {
        printf("%s\n", toString().c_str());
    }

protected:
    PrimitiveType _type;
    std::string _string;
    unsigned long _integer;

};

#endif
