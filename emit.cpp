
/*
 * Simply used to emit LLVM IR to build the stack machine.
 */

#include <cstdio>
#include <string>

#define PAGE 4096

typedef unsigned long Word;

Word stack[PAGE] = {0};
Word* top = stack;

void emitPrintString ()
{
    printf("%s\n", (char*) *(top - 1));
}

void emitStackPushHeapAllocation ()
{
    char *s = (char*) malloc(64);
    if (!s)
        exit(1);
    strcpy(s, "leroy");
    *top = (Word) s;
    top++;
}

class Foo
{
protected:
    static std::string _string;

public:
    Foo () {}
    static void bar ()
    {
        _string = "lol";
    }
};

void
emitStaticStringAndStringPush ()
{
    std::string s = "foobar";
    *top = (Word) &s;
    top++;
    printf("%s\n", ((std::string*) stack[0])->c_str());
}

void
emitPushInteger ()
{
    *top = 33;
    top++;
    *top = 72;
    top++;
    printf("%lu\n", stack[0]);
    printf("%lu\n", stack[1]);
    printf("%p != %p\n", stack, top);
}

void
emitStaticCall ()
{
    Foo::bar();
}

int
main ()
{
    return 0;
}
