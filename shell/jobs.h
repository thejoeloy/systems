#ifndef JOBS_H
#define JOBS_H

#include <unistd.h>
#include "defines.h"

Job* addJob(pid_t pgid, int state, const char *cmdLine);
void removeJob(pid_t pgid);
Job* getJob(pid_t pgid);
void printJobs();

#endif // JOBS_H
