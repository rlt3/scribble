#ifndef SCRIBBLE_LLVM
#define SCRIBBLE_LLVM

#include <string>

class LLVM
{
public:
    LLVM ();
    ~LLVM ();

    /* Compile and add the IR to the global list of definitions */
    void defineIR (std::string ir);

    /* Compile and execute IR and then execute the function `name` */
    void execute (std::string name, std::string ir);

    unsigned long* getStack ();

private:
    void *context;
};

#endif
