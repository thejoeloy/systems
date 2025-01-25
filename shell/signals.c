#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include "defines.h"
#include "jobs.h"
#include "signals.h"

// Signal handler to
void signalHandlerCHLD(int sig) {
    pid_t pid;
    int status;

    if ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Job* job = getJob(pid);
        if (job) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                removeJob(pid);
            }
        }
    }
}

// Signal handler to handle Ctrl-C keyboard interrupts and stop signals
void signalHandlerTSTP(int sig) {
    if (fgPGID != 0) {
        if (kill(-fgPGID, SIGTSTP) < 0) {
            perror("kill (SIGTSTP)");
        }
    }
}

// Signal handler to handle Ctrl-Z keyboard interrupts and interrupt signals
void signalHandlerINT(int sig) {
    if (fgPGID != 0) {
        if (kill(-fgPGID, SIGINT) < 0) {
            perror("kill (SIGINT)");
        }
    }
}

// Used to setup signal handlers for the shell
void setupSignalHandlers() {
    signal(SIGCHLD, signalHandlerCHLD);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, signalHandlerTSTP);
    signal(SIGINT, signalHandlerINT);
}
