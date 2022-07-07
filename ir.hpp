#ifndef SCRIBBLE_IR
#define SCRIBBLE_IR

#include <string>
#include <vector>

class IR {
public:
    IR ()
    {}

    IR (std::string ir)
    {
        std::string line = "";
        for (char c : ir) {
            line += c;
            if (c == '\n') {
                lines.push_back(line);
                line = "";
            }
        }
        lines.push_back(line);
    }

    std::string
    getString ()
    {
        std::string s = "";
        for (auto line : lines)
            s += line;
        return s;
    }

protected:
    std::vector<std::string> lines;
};

#endif
