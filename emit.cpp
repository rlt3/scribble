
/*
 * Simply used to emit LLVM IR to build the stack machine.
 */

#include <cstdio>
#include <string>

typedef unsigned long Word;

Word stack[4096] = {0};
Word* top = stack;

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
    static std::string s = "foobar";
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
