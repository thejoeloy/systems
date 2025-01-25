#include <stdio.h>
#include <stdlib.h>
#include "builtins.h"
#include "defines.h"
#include "executor.h"
#include "jobs.h"
#include "parser.h"
#include "signals.h"

Job *jobList = NULL;
int nextJID = 1;
pid_t fgPGID = 0;


int main() {
    
    char cmdLine[MAXLINE];
    setupSignalHandlers();
    // Loop for terminal evaluation
    while (1) {
        printf("joesh> ");
        fgets(cmdLine, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
 
        Eval(cmdLine);
    }
 }

