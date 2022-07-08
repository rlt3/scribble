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


--------------------------------------------------------------------------------

We are using LLVM for the JIT and native binary compiler. This allows us to
take advantage of thousands of man-hours in an already working system. Because
of this, there are drawbacks. First, we lose a little flexibility -- one such
example is updating functions but symbols (functions) cannot be updated in
LLVM's JIT easily (not that I have found), which calls for a numbering scheme.

However, most things are still on track:
    1) we can still design a custom ABI to fit the concatenative nature of
    stack-based languages
    2) we gain FFI and shared libraries for almost free
    3) any runtime-specific instrumentation, e.g.  number of function entries,
    exceptions, type information, can be gathered through FFI to some runtime
    object, e.g. LLVM IR calls some Frame method
    4) LLVM can compile a native executable for free

The plan is to build the primitives of the machine in LLVM IR directly. For
example, `add` would be a simple function that could be inlined defined in LLVM
IR. Any user defined procedures (called `descendents`), ultimately are just
combinations of primitives and other procedures.

Because the LLVM IR must independently be compiled into a native executable,
any structure must be embedded there and not into the runtime system. Therefore,
the entry point to the program is always a function which sets up the stack and
ABI before anything else. It can be the same function used in the JIT, but
a check must be made to verify setup hasn't already completed (for subsequent
execution) in the REPL.

ABI:
    - There are two stacks: the normal execution stack that gets updated when
    the CPU executes `push` or `pop` instructions and the data stack which holds
    all the values of the program.
    - Data stack needs to be allocated and the pointers to its head is placed on
    the execution stack.
    - Because all data related to the runtime of the program is on the data
    stack, doing runtime patching of the process is more feasible. For example,
    the regular stack may be freely trampled upon to do any of the numerous
    ancillary tasks of the runtime like reading input or replacing definitions.
    No non-Scribble task will ever touch the Scribble data-stack, therefore we
    may pause execution and allow for real-time replacement of functions without
    destroying any runtime variables and values.
    - All Scribble related stack manipulations occur on the data stack. Calls
    and other PC-modifying instructions within Scribble may use the execution
    stack.
    - Procedure arguments are hard to think about right now. What I do know is
    that we are semi-constrained to the C ABI because that is kinda how the
    LLVM IR is made. But because we can have two stacks, we can skirt some of
    the rules there. (The only exception is FFI calls, which demands us to save
    state of the stack.) My initial leanings are that arguments to a procedure
    are placed in registers like x86.
    - Heap allocation is something interesting to consider. I am thinking that
    heap allocation is simply allocating a small, runtime-bounds-checked stack.
    So, if we wanted dynamic strings, then we start by allocating some small
    stack and increased it as needed, but ultimately just pushing and popping
    characters onto the stack.
