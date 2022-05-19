#include "bytecode.hpp"
#include "data.hpp"

Data::~Data()
{
    delete payload;
}

Data::Data ()
    : type(DATA_NULL)
    , isExecutable(false)
    , payload(Payload(0))
{}

Data::Data (Bytecode bytecode)
    : type(DATA_CODE)
    , isExecutable(true)
    , payload(Payload(bytecode))
{}

Data::Data (unsigned long value)
    : type(DATA_INTEGER)
    , isExecutable(false)
    , payload(Payload(value))
{}

Data::Data (std::string str)
    : type(DATA_STR)
    , isExecutable(false)
    , payload(Payload(str))
{}

unsigned long
Data::integer ()
{
    return payload.value;
}

Bytecode
Data::bytecode ()
{
    return payload.bytecode;
}

std::string
Data::string ()
{
    return payload.str;
}
