#include "test.cpp"

#include "ir.hpp"
#include "irbuilder.hpp"
#include "procedure.hpp"
#include "runtime.hpp"

BEGIN();

TEST(basicIntegerStackCheck)
{
    Runtime runtime;

    IRBuilder bump1;
    bump1.pushInteger(33);
    bump1.retvoid();

    IRBuilder bump2;
    bump2.pushInteger(72);
    bump2.pushInteger(9);
    bump2.retvoid();

    Procedure p1("foo", 0, bump1.buildFunc("foo"));
    Procedure p2("foo", 0, bump2.buildFunc("foo"));

    runtime.executeProcedure(p1);
    runtime.executeProcedure(p2);

    auto stackptr = runtime.getStack();
    assert(stackptr[0] == 33);
    assert(stackptr[1] == 72);
    assert(stackptr[2] == 9);

    auto typestack = runtime.getTypestack();
    for (int i = 0; i < 3; i++) {
        assert(typestack.top() == PRM_INTEGER);
        typestack.pop();
    }
    assert(typestack.size() == 0);
};

END();
