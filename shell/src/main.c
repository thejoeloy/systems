#include <stdio.h>
#include "shell.h"

#define MAXARGS 128

int main() {

    char cmdline[MAXLINE];
    
    while(1) {
        printf("joesh> ");
        fgets(cmdline, MAXLINE, stdin);
        
        if (feof(stdin))
            exit(0);

        eval(cmdline);
    }
}
