// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <termios.h>

// Defines
#define MAXLINE 1024
#define MAXARGS 100

// Structs
typedef struct process {
    struct process *next;
    char **argv;
    pid_t pid;
    char completed;
    char stopped;
    int status;
} process;

typedef struct job {
    struct job *next;
    char *command;
    process *first_process;
    pid_t pgid;
    char notified;
    struct termios tmodes;
    int stdin, stdout, stderr;
} job;


// Global vars
extern char **environ;
job *first_job;
pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

// Function declarations
void init_shell();
void parse_line(char *buf, char **argv, int *bg);
int builtin_command(char **argv);
void eval(char *cmd_line);
void launch_process(process *p, pid_t pgid, int infile, int outfile, int errfile, int bg);
void launch_job(job *j, int bg);
void put_job_in_fg(job *j, int cont);
void put_job_in_bg(job *j, int cont);
int mark_process_status(pid_t pid, int status);
void update_status();
void wait_for_job(job *j);
int job_is_stopped(job *j);
int job_is_completed(job *j);
void format_job_info(job *j, const char *status);
void do_job_notification();
void free_job(job *j);
void mark_job_as_running(job *j);
void continue_job(job *j, int bg);
void print_process(process *p);
void print_job(job *j);
void setup_signal_handlers();
void signal_handler_CHLD();
void signal_handler_TSTP();
void signal_handler_INT();

// main loop
int main() {
    // Initialize shell
    init_shell();
    // Declare buffer
    char cmd_line[MAXLINE];
    char *argv[MAXARGS];

    // Setup signal handlers
    setup_signal_handlers();
    
    // Evaluation loop
    while (1) {
        printf("> ");
        fgets(cmd_line, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
        eval(cmd_line);
    }

    return 0;
}

// shell init
void
init_shell()
{
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);

        shell_pgid = getpid();
        if (setpgid(shell_pgid, shell_pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(1);
        }

        tcsetpgrp(shell_terminal, shell_pgid);
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

// parser
void
parse_line (char *buf, char **argv, int *bg)
{
    char* delim;
    int argc;

    // Replace trailing \n with space and ignore leading whitespace
    buf[strlen(buf) - 1] = ' ';
    while (*buf && (*buf == ' '))
        buf++;

    // Build argument list
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) // Ignore spaces
            buf++;
    }

    argv[argc] = NULL;

    // Determine if job should be foreground or background
    if ((*bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;

}

// built in commands
int
builtin_command (char **argv)
{
    // Quit
    if (!strcmp(argv[0], "quit"))
        exit(0);
    
    // Jobs
    if (!strcmp(argv[0], "jobs")) {
        job *j = first_job;
        while (j != NULL) {
            printf("[%d] ", j->pgid);

            if (j->first_process->stopped)
                printf("Stopped ");
            else
                printf("Running ");

            printf("%s\n", j->command);
            j = j->next;
        }
        return 1;
        
    }
    
    // Background
    if (!strcmp(argv[0], "bg")) {
        return 1;
    }
    
    // Foreground
    if (!strcmp(argv[0], "fg")) {
        return 1;
    }
    
    // Slay
    if (!strcmp(argv[0], "slay")) {
        return 1;
    }
    
    // Halt
    if (!strcmp(argv[0], "halt")) {
        return 1;
    }

    // Continue
    if (!strcmp(argv[0], "continue")) {
        return 1;
    }
    
    // cd
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] == NULL) {
            perror("cd: expected argument");
        }
        if(chdir(argv[1]) < 0) {
            printf("cd: %s is not a directory\n", argv[1]);
        }
        return 1;
    }
    
    // pwd
    if (!strcmp(argv[0], "pwd")) {
        char cwd[MAXLINE];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        else
            perror("pwd");
        fflush(stdout);
        return 1;
    }
    
    // echo
    if (!strcmp(argv[0], "echo")) {
        for (int i = 1; argv[i] != NULL; i++) {
            printf("%s", argv[i]);
            if (argv[i + 1] != NULL)
                printf(" ");
        }
        printf("\n");
        fflush(stdout);
        return 1;
    }

    return 0;
}

// executor
void
eval (char *cmd_line)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmd_line);

    parse_line(buf, argv, &bg);

    // Do nothing if there are no arguments
    if (argv[0] == NULL)
        return;

    // Check if command is built in shell command and execute it
    if (builtin_command(argv))
        return;

    
    // Setup process struct and launch process
    process *p = malloc(sizeof(process));
    p->argv = malloc((MAXARGS) * sizeof(char *));
    for (int i = 0; argv[i] != NULL; i++) {
        p->argv[i] = strdup(argv[i]);
    }
    p->argv[MAXARGS - 1] = NULL;
    p->completed = 0;
    p->stopped = 0;
    p->status = 0;
    p->next = NULL;
    
    // Initialize job struct
    job *j = malloc(sizeof(job));;
    j->first_process = p;
    j->pgid = 0;
    j->command = strdup(buf);
    j->stdin = STDIN_FILENO;
    j->stdout = STDOUT_FILENO;
    j->stderr = STDERR_FILENO;
    j->next = NULL;
     
    // Execute command
    //pid = fork();

    launch_job(j, bg);

    if (bg)
        fflush(stdout);
    /*
    if (pid == 0) {
        launch_job(&j, bg);
    }
    else if (pid > 0) {
        p->pid = pid;

        if (bg) {
            printf("[%d] %s\n", pid, argv[0]);
        }
        else {
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                fprintf(stderr, "waitfg: waitpid error\n");
                exit(0);
            }
        }
    }
    else {
        perror("fork");
        exit(1);
    }
    */
}

void
launch_process (process *p, pid_t pgid, int infile, int outfile, int errfile, int bg)
{
    //printf("launch process\n");
    pid_t pid = getpid();
    
    if (shell_is_interactive) {

        if (pgid == 0)
            pgid = pid;
        setpgid(pid, pgid);
        if (!bg)
            tcsetpgrp(shell_terminal, pgid);

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
    }
    
    if (infile != STDIN_FILENO) {
        dup2(infile, STDIN_FILENO);
        close(infile);
    }
    if (outfile != STDOUT_FILENO) {
        dup2(outfile, STDOUT_FILENO);
        close(outfile);
    }
    if (errfile != STDERR_FILENO) {
        dup2(errfile, STDERR_FILENO);
        close(errfile);
    }
    //print_process(p);
    execvp(p->argv[0], p->argv);
    perror("execvp");
    exit(0);
}

// jobs
void 
launch_job (job *j, int bg)
{
    //printf("launch job\n");
    process *p;
    pid_t pid;
    int fd[2], infile, outfile;

    infile = j->stdin;
    for (p = j->first_process; p; p = p->next) {
        // Setup file descriptors for process
        if (p->next) {
            if (pipe(fd) < 0) {
                perror("pipe");
                exit(1);
            }
            outfile = fd[1];
        }
        else
            outfile = j->stdout;
        // Fork child processes
        pid = fork();
        if (pid == 0)
            launch_process(p, j->pgid, infile, outfile, j->stderr, bg);
        else if (pid < 0) {
            perror("fork");
            exit(1);
        }
        else {
            p->pid = pid;
            if (shell_is_interactive) {
                if (!j->pgid)
                    j->pgid = pid;
                setpgid(pid, j->pgid);
            }
        }

        if (infile != j->stdin)
            close(infile);
        if (outfile != j->stdout)
            close(outfile);
        infile = fd[0];        
    }

    if (!shell_is_interactive)
        wait_for_job(j);
    else if (bg)
        put_job_in_bg(j, 0);
    else
        put_job_in_fg(j, 0);
    //print_job(j);
}

void
put_job_in_fg(job *j, int cont)
{
    //printf("put_job_in_fg\n");
    tcsetpgrp(shell_terminal, j->pgid);

    if (cont) {
        tcsetattr(shell_terminal, TCSADRAIN, &j->tmodes);
        if (kill(-j->pgid, SIGCONT) < 0)
            perror("kill (SIGCONT)");
    }

    wait_for_job(j);

    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &j->tmodes);
    tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);
}

void
put_job_in_bg(job *j, int cont)
{
    //printf("put job in bg\n");
    if (cont)
        if (kill(-j->pgid, SIGCONT) < 0)
            perror("kill (SIGCONT)");
}

int
mark_process_status (pid_t pid, int status)
{
    //printf("mark process status\n");
    job *j;
    process *p;

    if (pid > 0) {
        for (j = first_job; j; j = j->next)
            for (p = j->first_process; p; p = p->next)
                if (p->pid == pid) {
                    p->status = status;
                    if (WIFSTOPPED(status))
                        p->stopped = 1;
                    else {
                        p->completed = 1;
                        if (WIFSIGNALED(status))
                            fprintf(stderr, "%d: Terminated by signal %d.\n", (int)pid, WTERMSIG(p->status));
                    }
                    return 0;
                }
        fprintf(stderr, "No child process %d.\n", pid);
        return -1;
    }
    else if (pid == 0 || errno == ECHILD)
        return -1;
    else {
        perror("waitpid");
        return -1;
    }
}

void
update_status ()
{
    //printf("update status\n");
    int status;
    pid_t pid;

    do 
        pid = waitpid(WAIT_ANY, &status, WUNTRACED|WNOHANG);
    while (!mark_process_status(pid, status));
}

void
wait_for_job (job *j)
{
    //printf("wait for job\n");
    int status;
    pid_t pid;

    do
        pid = waitpid(WAIT_ANY, &status, WUNTRACED);
    while (!mark_process_status(pid, status) && !job_is_stopped(j) && !job_is_completed(j));
}

int
job_is_stopped (job *j)
{
    //printf("job is stopped\n");
    for (process *p = j->first_process; p; p = p->next) {
        if (!p->stopped)
            return 0;
    }
    return 1;
}

int
job_is_completed (job *j)
{
    //printf("job is completed\n");
    for (process *p = j->first_process; p; p->next) {
        if (!p->completed)
            return 0;
    }
    return 1;
}

void
format_job_info (job *j, const char *status)
{
    fprintf(stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

void 
do_job_notification ()
{
    //printf("do job notification\n");
    job *j, *jlast, *jnext;
    
    update_status();

    jlast = NULL;

    for (j = first_job; j; j = jnext) {
        jnext = j->next;

        if (job_is_completed(j)) {
            format_job_info(j, "completed");
            if (jlast)
                jlast->next = jnext;
            else
                first_job = jnext;
            free_job(j);
        }

        else if (job_is_stopped(j) && !j->notified) {
            format_job_info(j, "stopped");
            j->notified = 1;
            jlast = j;
        }

        else
            jlast = j;
    }
}

void
free_job (job *j)
{
    //printf("free job\n");
    process *p = j->first_process;
    while (p) {
        process *pnext = p->next;
        free(p->argv);
        free(p);
        p = pnext;
    }

    free(j);
}

void
mark_job_as_running (job *j)
{
    //printf("mark job as running\n");
    process *p;

    for (p = j->first_process; p; p = p->next)
        p->stopped = 0;
    j->notified = 0;
}

void
continue_job (job *j, int bg)
{
    //printf("continue job\n");
    mark_job_as_running(j);
    if (bg)
        put_job_in_bg(j, 1);
    else
        put_job_in_fg(j, 1);
}

void
print_process(process *p)
{
    if (!p) {
        printf("NULL process\n");
        return;
    }
    
    printf("Process PID: %d\n", p->pid);
    printf("Completed: %s\n", p->completed ? "Yes" : "No");
    printf("Stopped: %s\n", p->stopped ? "Yes" : "No");
    printf("Status: %d\n", p->status);

    printf("Arguments: ");
    if (p->argv) {
        for (int i = 0; p->argv[i] != NULL; i++) {
            printf("%s ", p->argv[i]);
        }
    }
    printf("\n");
}

void
print_job (job *j)
{
    if (!j) {
        printf("NULL job\n");
        return;
    }

    printf("Job Command: %s\n", j->command ? j->command : "(null)");
    printf("PGID: %d\n", j->pgid);
    printf("Notified: %s\n", j->notified ? "Yes" : "No");
    printf("File Descriptors: stdin=%d, stdout=%d, stderr=%d\n",
           j->stdin, j->stdout, j->stderr);

    printf("Processes in job:\n");
    for (process *p = j->first_process; p; p = p->next) {
        print_process(p);
        printf("----\n");
    }
}

void 
setup_signal_handlers() 
{
    signal(SIGCHLD, signal_handler_CHLD);
    signal(SIGTSTP, signal_handler_TSTP);
    signal(SIGINT, signal_handler_INT);
}

void 
signal_handler_CHLD (int sig) 
{
    do_job_notification();
}

void 
signal_handler_TSTP (int sig) 
{
    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);

    if (fg_pgid != shell_pgid) 
        if (kill(-fg_pgid, SIGTSTP) < 0)
            perror("kill (SIGTSTP)");
}

void 
signal_handler_INT (int sig) 
{
    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);

    if (fg_pgid != shell_pgid)
        if (kill(-fg_pgid, SIGINT) < 0)
            perror("kill (SIGINT)");
}
