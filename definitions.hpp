#ifndef SCRIBBLE_DEFINITIONS
#define SCRIBBLE_DEFINITIONS

typedef enum {
    REGNULL = 0,
    REG1,
    REG2,
    REG3,
    REGCOUNT
} Register;

typedef enum {
    OP_NULL,
    OP_HALT,
    OP_MOVE,
    OP_POINTER,
    OP_LOAD,
    OP_PUSH,
    OP_POP,
    OP_PRINT,
    OP_ADD,
} Operator;

void fatal (const char* format, ...);

#endif
