#ifndef DEFINES_H
#define DEFINES_H

#include <unistd.h>
#include <sys/wait.h>

#define MAXLINE 128
#define MAXARGS 64
#define FG 1
#define BG 2
#define ST 3

typedef struct Job {
    int jid;
    pid_t pgid;
    char cmdLine[MAXLINE];
    int state;
    struct Job *next;
} Job;

extern Job *jobList;
extern int nextJID;
extern pid_t fgPGID;
extern char **environ;

#endif // DEFINES_H
