#ifndef SCRIBBLE_DATA
#define SCRIBBLE_DATA

#include <cstdio>
#include <string>
#include <cassert>
#include "bytecode.hpp"
#include "primitive.hpp"

typedef enum {
    DATA_NULL,
    DATA_PRIMITIVE,
    DATA_CODE
} DataType;

/*
 * Represents a Word of memory on the stack. But since we have a runtime, we
 * can include stuff like type information.
 *
 * TODO: Should perhaps think about an immutable implementation so that values
 * always remain exactly as they were first set. We want this because we don't
 * want to deal with that on the stack. Constructors and Copy-Constructors
 * only.
 */
struct Data
{
    Data ()
        : _type(DATA_NULL)
        , _primitive(Primitive())
        , _bytecode(Bytecode())
        , _is_executable(false)
    {}

    Data (unsigned long integer)
        : _type(DATA_PRIMITIVE)
        , _primitive(Primitive(integer))
        , _bytecode(Bytecode())
        , _is_executable(false)
    {}

    Data (Bytecode bytecode)
        : _type(DATA_CODE)
        , _primitive(Primitive())
        , _bytecode(bytecode)
        , _is_executable(true)
    {}

    DataType
    type () const
    {
        return _type;
    }

    bool
    isExecutable () const
    {
        return _is_executable;
    }

    void
    assign (Data& data)
    {
        _type = data._type;
        _primitive = data._primitive;
        _bytecode = data._bytecode;
        _is_executable = data._is_executable;
    }

    void
    assign (Primitive primitive)
    {
        _type = DATA_PRIMITIVE;
        _primitive = primitive;
    }

    void
    assign (Bytecode bytecode)
    {
        _type = DATA_CODE;
        /*
         * TODO instead mark parts of the stack executable during creation.
         */
        _is_executable = true;
        _bytecode = bytecode;
    }

    Primitive
    primitive ()
    {
        assert(_type == DATA_PRIMITIVE);
        return _primitive;
    }

    Bytecode
    bytecode ()
    {
        assert(_type == DATA_CODE);
        return _bytecode;
    }

    std::string
    toString ()
    {
        switch (_type) {
            case DATA_CODE: return "<code>";
            case DATA_PRIMITIVE: return _primitive.toString();

            case DATA_NULL:
            default:
                return "NULL";
        }
    }

protected:
    DataType _type;
    Primitive _primitive;
    Bytecode _bytecode;
    bool _is_executable;
};

#endif
