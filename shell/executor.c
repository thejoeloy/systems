#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "builtins.h"
#include "defines.h"
#include "executor.h"
#include "jobs.h"
#include "parser.h"


void Eval(char *cmdLine) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdLine);
    bg = ParseLine(buf, argv);

    // Do nothing if there are no arguments
    if (argv[0] == NULL) return;

    // If command is built in command, execute it
    if (BuiltInCommand(argv)) return;

    // Search for executable (if possible) and execute it
    char *command = argv[0];
    char *fullPath = NULL;

    // If command is not executable, search for it in path
    if (command[0] != '/') {
        char *pathEnv = strdup(getenv("PATH"));
        if (pathEnv == NULL) {
            printf("No PATH variable found\n");
            return;
        }

        char *path = strtok(pathEnv, ":");
        while (path != NULL) {
            fullPath = malloc(strlen(path) + strlen(command) + 2);
            if (fullPath == NULL) {
                perror("malloc");
                exit(1);
            }

            sprintf(fullPath, "%s/%s", path, command);

            if (access(fullPath, X_OK) == 0) break;

            free(fullPath);
            fullPath = NULL;
            path = strtok(NULL, ":");
        }
    }
    // If command is executable, simply make the path the command
    else {
        fullPath = command;
    }

    // Execute the command
    argv[0] = fullPath;

    if ((pid = fork()) == 0) {
        
        if (execve(argv[0], argv, environ) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            exit(0);
        }
        free(fullPath);
    }

    if (!bg) {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            fprintf(stderr, "waitfg: waitpid error\n");
            exit(0);
        }
    }
    return;
}
