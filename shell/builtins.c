#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "builtins.h"
#include "defines.h"
#include "jobs.h"


// Evaluate the command line
int BuiltInCommand(char **argv) {
    // Exit command
    if (!strcmp(argv[0], "exit")) {
        // Check if a properly formatted error code was entered in
        if (argv[1] != NULL) {
            char *endPtr;
            long exitStatus = strtol(argv[1], &endPtr, 10);
            if (*endPtr != '\0' || endPtr == argv[1]) {
                fprintf(stderr, "exit: invalid exit status: %s\n", argv[1]);
                return 1;
            }
            exit((int)exitStatus);
        }
        // Exit with 0 if no exit status is provided
        else {
            exit(0);
        }
    }

    // Quit command
    if (!strcmp(argv[0], "quit"))
         exit(0);

    // Background command
    if (!strcmp(argv[0], "&"))
         return 1;

    // jobs command - get a list of the jobs
    if (!strcmp(argv[0], "jobs")) {
         //printJobs();
         return 1;
    }

    // foreground command - bring background processes back to the front
    if (!strcmp(argv[0], "fg")) {
         return 1;
    }

    // background command - Continue a background command
    if (!strcmp(argv[0], "bg")) {
         return 1;
    }

    // slay command - kill a set of processes in a pipeline
    if (!strcmp(argv[0], "slay")) {
         return 1;
    }

    // halt command - stop a process
    if (!strcmp(argv[0], "halt")) {
         return 1;
    }

    // continue command - resume a process
    if (!strcmp(argv[0], "cont")) {
         return 1;
    }

    // cd command
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        }
        else if (chdir(argv[1]) < 0) {
            perror("cd");
        }
        return 1;
    }

    // pwd command
    if (!strcmp(argv[0], "pwd")) {
        char cwd[MAXLINE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
        else perror("pwd");
        return 1;
    }

    // echo command
    if (!strcmp(argv[0], "echo")) {
        if (argv[1] == NULL) fprintf(stderr, "echo: expected argument");
        for (int i = 1; argv[i] != NULL; i++) {
            printf("%s", argv[i]);
            if (argv[i + 1] != NULL) printf(" ");
        }
        printf("\n");
        return 1;
    }

    return 0;
}

