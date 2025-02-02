# Simple Shell
Contains a simple implementation of a shell. The code draws from Computer Systems: A Programmers Perspective by Bryant and 
O'Hallaron. The job control is heavily influenced by the glibc documentation. The functionality is inspired by the Stanford Shell
assignment from CS110. The shell is capable of pipelining and redirection, signal interrupts, and the built in commands such as
echo, ls, cd, jobs, fg, background, slay, halt, and continue. The slay, halt, and continue commands work at the job level instead
of the process level. I am able to use keyboard interrupts to stop and terminate a process using ctrl-z and ctrl-c. I am able to
redirect from both standard in and standard out as well.

The system wasnt made to be portable, but to be a learning experience with multiprocessing. I only guarantee correctness for my Gentoo Base System Release 2.17, but it should generally work on other Linux machines. 
