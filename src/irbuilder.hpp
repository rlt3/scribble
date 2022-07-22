#ifndef SCRIBBLE_IRBUILDER
#define SCRIBBLE_IRBUILDER

#include "ir.hpp"

/*
 * Interactively build and output an IR object.
 */
class IRBuilder {
public:
    IRBuilder () : _tmp(1)
    {
    }

    void
    pushInteger (int number)
    {
        auto n = std::to_string(number);

        /*
         * Get the current top of the stack and write to it. Then calculate the
         * next stack location and save it as the top.
         */
        auto curr = tmpvar();
        auto next = tmpvar();
        add("%" + curr + " = load i64*, i64** @top, align 8");
        add("store i64 " + n + ", i64* %" + curr + ", align 8");
        add("%" + next + " = getelementptr inbounds i64, i64* %" + curr + ", i32 1");
        add("store i64* %" + next + ", i64** @top, align 8");

        /* 
         * Here or somewhere near here is where we can optionally add a call to
         * some internally defined method to book keep type values on the
         * stack.
         */
        add("call void @typestack_pushInteger()");
    }

    void
    pushString (std::string str)
    {
        /*
         * TODO:
         * Trying to understand how I want to handle non-primitive types. I
         * think heap allocating everything is fine. So, behind-the-scenes, we
         * handle memory just like std::string or std::vector may. Therefore,
         * all non-primitive types would simply be a pointer. This pointer can
         * be aliased as the pointed-to object can be the explicit value of the
         * pointer or a runtime structure.
         */
    }

    void
    popInteger ()
    {
    }

    void
    load ()
    {
    }

    void
    move ()
    {
    }

    void
    retnum (std::string number)
    {
       add("ret i32 " + number);
    }

    void
    retvoid ()
    {
       add("ret void");
    }

    IR
    buildFunc (std::string name)
    {
        std::string s = "define void @" + name + "() {\n";
        for (auto line : _lines)
            s += line;
        s += "}\n";
        return IR(s);
    }

protected:
    std::vector<std::string> _lines;
    unsigned _tmp;

    std::string
    tmpvar ()
    {
        std::string s = std::to_string(_tmp);
        _tmp++;
        return s;
    }

    inline void
    add (std::string s)
    {
        _lines.push_back("\t" + s + "\n");
    }
};

#endif
