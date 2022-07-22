#ifndef SCRIBBLE_FRAME
#define SCRIBBLE_FRAME

#include <map>
#include <string>
#include "definitions.hpp"

/*
 * Look up definitions within a frame, e.g. local variable, arguments to the
 * function, etc.
 */

class Frame {
public:

protected:
    std::map<std::string, Register> arguments;

private:
};

#endif
