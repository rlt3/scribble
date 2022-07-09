#ifndef SCRIBBLE_IR
#define SCRIBBLE_IR

#include <string>
#include <vector>

class IR {
public:
    /* Create from a predefined string */
    IR (std::string ir)
        : _interactive(false)
    {
        std::string line = "";
        for (char c : ir) {
            line += c;
            if (c == '\n') {
                _lines.push_back(line);
                line = "";
            }
        }
        _lines.push_back(line);
    }

    /* Create and build interactively using the methods below */
    IR ()
        : _interactive(true)
    {}


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

    std::string
    getString ()
    {
        std::string s = "";
        if (_interactive)
            s += "define void @main () {\n";
        for (const auto line : _lines)
            s += line;
        if (_interactive)
            s += "}\n";
        return s;
    }

protected:
    bool _interactive;
    std::vector<std::string> _lines;

    inline void
    add (std::string s)
    {
        _lines.push_back("\t" + s + "\n");
    }
};

#endif
