#include <string.h>
#include "parser.h"

// Parse the command line and build the argv array
int ParseLine(char *buf, char **argv) {
    char *delim;    // Points to the first space delimeter
    int argc;       // Number of the args
    int bg;         // Flag for whether a process should be background or not

    buf[strlen(buf) - 1] = ' '; // replace trailing \n with space
    while (*buf && (*buf == ' '))   // ignore leading spaces
        buf++;

    // Build the argument list
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' '))   // ignore spaces
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)  // Ignore blank lines
        return 1;

    // Determines whether or not the job should run in the background
    if ((bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}

