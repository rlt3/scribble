#ifndef SCRIBBLE_DATA
#define SCRIBBLE_DATA

#include <string>

struct Bytecode;
struct Payload;

typedef enum {
    DATA_NULL,
    DATA_STR,
    DATA_CODE,
    DATA_INTEGER,
} DataType;

struct Payload
{
    std::string str;
    unsigned long value;
    Bytecode bytecode;

    Payload (std::string s) : str(s) {}
    Payload (unsigned long v) : value(v) {}
    Payload (Bytecode bc) : bytecode(bc) {}
};

/*
 * Represents a Word of memory on the stack. But since we have a runtime, we
 * can include stuff like type information.
 */
class Data
{
public:
    DataType type;
    bool isExecutable;

    Data ();
    ~Data ();
    Data (Bytecode bytecode);
    Data (unsigned long value);
    Data (std::string str);

    unsigned long integer();
    Bytecode bytecode();
    std::string string();

protected:
    Payload payload;
};

#endif
