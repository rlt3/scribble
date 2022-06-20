# Design Goals

1) To have an integrated environment for development between a running process
and source code. This includes a REPL with some sort of source code viewer for
displaying particular definitions.

2) To be able to develop a program while you're using it. The environment above
should be reactive enough that you can define new procedures or redefine old
ones without the program crashing.

3) To have an integrated runtime that can easily output independent executables
with no runtime overhead. The runtime overhead being the integrated environment
above.

4) The above is carried out by academic techniques similar to a JIT where the
interpreted code is compiled natively before being executed. Not a traditional
intepreter.

5) Runtime information should still be available. I am currently recommending to
have an abstract Bytecode value with which to define procedures that are then
compiled into the native language. Meaning, the bytecode could be intepreted
(and is now), but can instead also serve to hold, e.g., type information,
argument counts, etc.

6) C's ABI for foreign functions.

7) A simple Lisp as the top-level language gives us power to easily create
a traditional C-like language later.
