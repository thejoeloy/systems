# Systems Programming
This repo contains a variety of the core systems programs and utilities that go into a computer system. The goal is to implement simple versions of all of the useful hardware and software components that go into a computer. The implementations will
focus on open source components with licenses that encourage privacy, freedom, and collobaration. Namely, hardware components will target the RISC-V architecture, and the software components will target Unix like systems such as Linux and BSD.

## Components

### Hardware

CPU: Contains an implementation of the 5 stage pipelined RISC-V processor described in Computer Organization and Design by Patterson and Hennessey
The processor implements the RV32I instructions. As of now, the processor is capable of running at 50 MHz on my Zedboard

Cache: TODO! Contains an implementation of the direct mapped cache described in Computer Organization and Design by Patterson and Hennessey.
The cache has a write-back scheme that uses write allocate, has a block size of 4, uses 32 bit addresses, and has a valid and dirty bit per block.

Virtual Memory Hardware: TODO! Contains an implementation of the hardware components and features that go into virtual memory. My implementation
is based on the design described in Computer Organization and Design by Hennessey and Patterson. 

### Software

Heap Allocator: Contains implementations of the implicit free list and explicit free list heap allocator described in Computer Systems: A
Programmer's Perspective by Bryant and O'Hallaron. Both allocators have a scheme in which coalescing is performed immediately. The explicit free
list heap allocator maintains its free list in address order. 

Shell: TODO! Contains an implementation of a shell that is based on both the shell skeleton provided in Computer Systems: A Programmer's Perspective 
and the Stanford CS110 assignment Stanford Shell. This program is capable of running built-in system commands, pipelining, redirection, signal
handling, background tasks, and keyboard interrupts.

Filesystem: TODO! Contains an implementation of the Unix v6 filesystem described in Principles of Computer System Design by Saltzer and Kaashoek.

Virtual Memory Manager: TODO! Contains implementations of the the virtual memory management features described in Computer Systems: A Programmer's
Perspective.

I/O: A file containing the code for File I/O described in the Robust I/O section in Computer Systems: A Programmer's Perspective. 

Networking: Contains an implementation of the tiny server described in Computer Systems: A Programmer's Perspective. 

System Utilities: Contains implementations of the system utilities cat, ls, printenv, sort, tail, uniq, and which.


