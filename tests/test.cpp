#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <signal.h>
#include <unistd.h>
#include <functional>
#include <map>
    
static const char *_test_name = NULL;

#define COLOR_RESET "\e[0m"
#define COLOR_RED   "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define CROSS       "\u2718"
#define CHECKMARK   "\u2714"

void
report_fail (int signum)
{
    printf(COLOR_RED CROSS COLOR_RESET " - %s\n", _test_name);
    exit(1);
}


#define BEGIN() \
    int \
    main (int argc, char **argv) \
    { \
        std::map<const char*, std::function<void()>> _tests;

#define TEST(name) \
        _tests[#name] = []()

#define END() \
        signal(SIGABRT, report_fail); \
        for (auto test : _tests) { \
            _test_name = test.first; \
            test.second(); \
            printf(COLOR_GREEN CHECKMARK COLOR_RESET " - %s\n", _test_name); \
        } \
        \
        return 0; \
    }
