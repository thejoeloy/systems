#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "defines.h"
#include "jobs.h"

Job* addJob(pid_t pgid, int state, const char *cmdLine) {
    return NULL;
}

void removeJob(pid_t pgid) {
    return;
}

Job* getJob(pid_t pgid) {
    return NULL;
}

void printJobs() {
    return;
}


