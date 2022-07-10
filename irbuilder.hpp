#ifndef SCRIBBLE_IRBUILDER
#define SCRIBBLE_IRBUILDER

#include "ir.hpp"

/*
 * Interactively build and output an IR object.
 */
class IRBuilder {
public:
    IRBuilder ()
    {
    }

    void
    push (std::string number)
    {
        /*
         * Load top pointer, write to that location. Get the next top and write
         * that into the global `top` variable.
         */
        add("%old = load i64*, i64** @top, align 8");
        add("store i64 " + number + ", i64* %old, align 8");
        add("%new = getelementptr inbounds i64, i64* %old, i32 1");
        add("store i64* %new, i64** @top, align 8");
    }

    void
    pop ()
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

    inline void
    add (std::string s)
    {
        _lines.push_back("\t" + s + "\n");
    }
};

#endif
