Pura - Experimental Java Virtual Machine
========================================

Pura is a cleanroom implementation of a Java Virtual Machine written in ANSI C-89. It is intended as a "Learners JVM" for people who want to know what happens behind the curtain when certain lines of code are executed, and that without drowning them in complexity, which unfortunately is what most of the modern implementations do. So the intent was to write a readable and self explanatory VM, so that a developer can easily step or debug through and understand what is happening. The data structures have been explicitly choosen to be simple, too, which means that most of the time simple lists have been used where other structures or algorithms would have offered better performance.

I wrote Pura as part of my diploma thesis in Computer Science at the University of Applied Sciences Cologne. The German thesis paper can be found in the "doc" folder.

Please note that Pura is far from complete. It basically is just the (mostly complete) loader and execution engine (interpreter) part of a JVM. Other important parts like a garbage collector or the verifier are missing and the interpreter is limited to one thread. Also there is no test rig, which means there may be an unknown number of bugs in the implementation. Many events that normally generate exceptions currently generate errors instead, because exception handling in the interpreter itself (in the C source) is not implemented. Throwing exceptions in interpreted code works fine though.

Pura uses its own Java class library, which only implements a very limited set of base classes and even those are far from completely implemented. The intention here was to get simple applications and command line output going, which is working fine. Note that Pura uses a proprietary native API. For methods marked as "native" there has to be explicit support within native.c.

Pura has been tested on PowerPC and x86 processors in 32 bit mode under OS X. It should also work under Windows and Linux. 64 bit modes have not been tested and may not work.


Usage:

Pura is supplied as an Xcode project and can easily be opened and used after checkout. Initially the project is configured to execute the "Hello World" example contained in the "testcases" folder. These, as well as other command line arguments, can be changed in the scheme editor. On the command line, the compiled binary can be used as a drop-in replacement for the  "java" command. Supported arguments are documented in puraMain.c.

Please note that the Java files in the "Lib" "Testcases" folders in Xcode have to be manually recompiled when changed.

Currently there is no makefile, but on the command line the project can simply be compiled using "gcc -o pura *.c" or similar.
