#ifndef SCRIBBLE_PRIMITIVE
#define SCRIBBLE_PRIMITIVE

#include <cassert>
#include <string>

typedef enum {
    PRM_NULL,
    PRM_STRING,
    PRM_INTEGER,
    NUM_PRM
} PrimitiveType;

struct Primitive
{
    Primitive () : _type(PRM_NULL) {}
    Primitive (std::string s) : _type(PRM_STRING), _string(s) {}
    Primitive (unsigned long v) : _type(PRM_INTEGER), _integer(v) {}

    PrimitiveType
    type ()
    {
        return _type;
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

    void
    print ()
    {
        switch (_type) {
            case PRM_STRING:
                printf("string(%s)\n", _string.c_str());
                break;

            case PRM_INTEGER:
                printf("<integer>(0x%08lx)\n", _integer);
                break;

            default:
                printf("NULL()\n");
                break;
        }
    }

protected:
    PrimitiveType _type;
    std::string _string;
    unsigned long _integer;

};

#endif
