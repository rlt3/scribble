#ifndef SCRIBBLE_DEFINITIONS
#define SCRIBBLE_DEFINITIONS

#define REPL_INPUT_STR  "< "
#define REPL_OUTPUT_STR "> "
#define REPL_INFO_STR   "| "

#define REPL_SYMBOL "::repl::"
#define NUM_ARG_REGISTERS 3

typedef enum {
    REGNULL = 0,
    REG1,
    REG2,
    REG3,
    REGCALL,
    REGBASE,
    REGCOUNT
} Register;

typedef enum {
    OP_NULL,
    OP_HALT,
    OP_MOVESTR,
    OP_MOVEINT,
    OP_MOVESYM,
    OP_LOADSTR,
    OP_LOADINT,
    OP_LOADSYM,
    OP_PUSH,
    OP_POP,
    OP_CALL,
    OP_RET,
    OP_ADD,
    OP_PRINT,
} Operator;

//static std::string
//registerString (Register reg)
//{
//    switch (reg) {
//        case REGNULL:  return "NULL"; break;
//        case REG1:     return "REG1"; break;
//        case REG2:     return "REG2"; break;
//        case REG3:     return "REG3"; break;
//        case REGCALL:  return "REGCALL"; break;
//        case REGBASE:  return "REGBASE"; break;
//        case REGCOUNT: return "REGCOUNT"; break;
//        default:
//            return "!-! BAD REGISTER !-!";
//    }
//}
//
//static std::string
//operatorString (Operator op)
//{
//    switch (op) {
//        case OP_NULL:    return "NULL"; break;
//        case OP_HALT:    return "HALT"; break;
//        case OP_MOVEINT: return "MOVEINT"; break;
//        case OP_MOVESTR: return "MOVESTR"; break;
//        case OP_MOVESYM: return "MOVESYM"; break;
//        case OP_LOADINT: return "LOADINT"; break;
//        case OP_LOADSTR: return "LOADSTR"; break;
//        case OP_PUSH:    return "PUSH"; break;
//        case OP_POP:     return "POP"; break;
//        case OP_CALL:    return "CALL"; break;
//        case OP_RET:     return "RET"; break;
//        case OP_ADD:     return "ADD"; break;
//        case OP_PRINT:   return "PRINT"; break;
//        default:
//            return "!-! BAD OP !-!";
//    }
//}

#endif
