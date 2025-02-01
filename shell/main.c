// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>

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
void add_job(job *j);
void remove_job(job *j);
int job_is_stopped(job *j);
int job_is_completed(job *j);
void free_job(job *j);
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

    // Setup signal handlers
    setup_signal_handlers();
    
    // Evaluation loop
    while (1) {
        fflush(stdout);
        printf("> ");
        fflush(stdout);
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
    // Quit and Exit
    if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit"))
        exit(0);
    
    // Jobs
    if (!strcmp(argv[0], "jobs")) {
        job *j = first_job;
        while (j != NULL) {
            printf("[%d] ", j->pgid);

            if (job_is_stopped(j))
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
        if (argv[1] == NULL) {
            perror("bg: expected job number\n");
            return 1;
        }

        char *endptr;
        int job_num = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            perror("bg: invalid job number\n");
            return 1;
        }

        job *j = first_job;
        while (j != NULL && j->pgid != job_num)
            j = j->next;

        if (j == NULL) {
            perror("bg: no such job");
            return 1;
        }

        put_job_in_bg(j, 1);
        return 1;

    }
    
    // Foreground
    if (!strcmp(argv[0], "fg")) {
        if (argv[1] == NULL) {
            perror("fg: expected job number\n");
            return 1;
        }

        char *endptr;
        int job_num = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            perror("fg: invalid job number\n");
            return 1;
        }

        job *j = first_job;
        while (j != NULL && j->pgid != job_num)
            j = j->next;

        if (j == NULL) {
            perror("fg: no such job");
            return 1;
        }

        put_job_in_fg(j, 1);
        return 1;
    }
    
    // Slay
    if (!strcmp(argv[0], "slay")) {
        if (argv[1] == NULL) {
            fprintf(stderr, "%s: expected job number\n", argv[0]);
            return 1;
        }

        char *endptr;
        int job_num = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            fprintf(stderr, "%s: invalid job number\n", argv[0]);
            return 1;
        }

        job *j = first_job;
        while (j != NULL && j->pgid != job_num)
            j = j->next;

        if (j == NULL) {
            fprintf(stderr, "%s: no such job [%d]\n", argv[0], job_num);
            return 1;
        }

        if (kill(-j->pgid, SIGKILL) < 0) {
            perror("kill (SIGKILL)");
            return 1;
        }

        
        tcsetpgrp(shell_terminal, shell_pgid);
        printf("> ");
        fflush(stdout);
        
        
        return 1;
    }
    
    // Halt
    if (!strcmp(argv[0], "halt")) {
        if (argv[1] == NULL) {
            fprintf(stderr, "%s: expected job number\n", argv[0]);
            return 1;
        }

        char *endptr;
        int job_num = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            fprintf(stderr, "%s: invalid job number\n", argv[0]);
            return 1;
        }

        job *j = first_job;
        while (j != NULL && j->pgid != job_num)
            j = j->next;

        if (j == NULL) {
            fprintf(stderr, "%s: no such job [%d]\n", argv[0], job_num);
            return 1;
        }

        if (kill(-j->pgid, SIGTSTP) < 0) {
            perror("kill (SIGTSTP)");
            return 1;
        }

        
        tcsetpgrp(shell_terminal, shell_pgid);
        printf("> ");
        fflush(stdout);
        
        return 1;
    }
    
    // Continue
    if (!strcmp(argv[0], "continue")) {
        if (argv[1] == NULL) {
            fprintf(stderr, "%s: expected job number\n", argv[0]);
            return 1;
        }

        char *endptr;
        int job_num = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            fprintf(stderr, "%s: invalid job number\n", argv[0]);
            return 1;
        }

        job *j = first_job;
        while (j != NULL && j->pgid != job_num)
            j = j->next;

        if (j == NULL) {
            fprintf(stderr, "%s: no such job [%d]\n", argv[0], job_num);
            return 1;
        }

        if (kill(-j->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
            return 1;
        }
        
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
    //pid_t pid;

    strcpy(buf, cmd_line);

    parse_line(buf, argv, &bg);

    // Do nothing if there are no arguments
    if (argv[0] == NULL)
        return;

    // Check if command is built in shell command and execute it
    if (builtin_command(argv))
        return;

    process *head = malloc(sizeof(process));
    head->argv = malloc(MAXARGS * sizeof(char *));
    head->completed = 0;
    head->stopped = 0;
    head->status = 0;
    head->next = NULL;

    process *curr = head;
    int argIndex = 0;

    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "|") == 0) {
            curr->argv[argIndex] = NULL;
            curr->completed = 0;
            curr->stopped = 0;
            curr->status = 0;

            curr->next = malloc(sizeof(process));
            curr = curr->next;
            curr->argv = malloc(MAXARGS * sizeof(char *));
            curr->completed = 0;
            curr->stopped = 0;
            curr->status = 0;
            curr->next = NULL;
            argIndex = 0;
        }
        else {
            curr->argv[argIndex++] = strdup(argv[i]);
        }
    }

    curr->argv[argIndex] = NULL;

    // Initialize job struct
    job *j = malloc(sizeof(job));;
    j->first_process = head;
    j->pgid = 0;
    j->command = strdup(cmd_line);
    j->stdin = STDIN_FILENO;
    j->stdout = STDOUT_FILENO;
    j->stderr = STDERR_FILENO;
    j->next = NULL;
    
    //print_job(j);

    launch_job(j, bg);

    if (bg)
        fflush(stdout);
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
        
        char *input_file = NULL, *output_file = NULL;
        int redirect_output = 0;
        int i = 0;
        while (p->argv[i] != NULL) {
            // Input file redirection
            if (strcmp(p->argv[i], "<") == 0) {
                
                input_file = p->argv[i + 1];
                if (input_file == NULL) {
                    fprintf(stderr, "Error: No file specified for input redirection\n");
                    return;
                }
                
                infile = open(input_file, O_RDONLY);
                if (infile < 0) {
                    perror("Open redirected input file");
                    return;
                }

                p->argv[i] = NULL;
                p->argv[i + 1] = NULL;
                break;
            }
            // Output file redirection
            if (strcmp(p->argv[i], ">") == 0 || strcmp(p->argv[i], ">>") == 0) {
                
                output_file = p->argv[i + 1];
                if (output_file == NULL) {
                    fprintf(stderr, "Error: No file specified for output redirection\n");
                    return;
                }


                if (strcmp(p->argv[i], ">") == 0) {
                    outfile = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                else {
                    outfile = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
                
                if (outfile < 0) {
                    perror("Open redirected output file");
                    return;
                }

                redirect_output = 1;
                p->argv[i] = NULL;
                p->argv[i + 1] = NULL;
                break;
            }
            i++;
        }

        // Setup file descriptors for process
        if (p->next || redirect_output) {
            if (pipe(fd) < 0) {
                perror("pipe");
                exit(1);
            }
            if (!redirect_output)
                outfile = fd[1];
        }
        else {
            outfile = j->stdout;
        }
        // Fork child processes
        pid = fork();
        if (pid == 0) {
            
            if (infile != STDIN_FILENO)
                if (dup2(infile, STDIN_FILENO) < 0) {
                    perror("dup2 infile");
                    exit(1);
                }
            
            if (outfile != STDOUT_FILENO)
                if (dup2(outfile, STDOUT_FILENO) < 0) {
                    perror("dup2 outfile");
                    exit(1);
                }
            
            launch_process(p, j->pgid, infile, outfile, j->stderr, bg);
        }
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

    add_job(j);

    if (bg)
        put_job_in_bg(j, 0);
    else
        put_job_in_fg(j, 0);
    //print_job(j);
}

void
put_job_in_fg (job *j, int cont)
{
    //printf("put_job_in_fg\n");
    tcsetpgrp(shell_terminal, j->pgid);

    if (cont) {
        tcsetattr(shell_terminal, TCSADRAIN, &j->tmodes);
        if (kill(-j->pgid, SIGCONT) < 0)
            perror("kill (SIGCONT)");
    }

    while (!job_is_completed(j) && !job_is_stopped(j)) {
        pause();
    }

    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &j->tmodes);
    tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);
}

void
put_job_in_bg (job *j, int cont)
{
    //printf("put job in bg\n");
    if (cont)
        if (kill(-j->pgid, SIGCONT) < 0)
            perror("kill (SIGCONT)");

}

void
add_job (job *j)
{
    j->next = first_job;
    first_job = j;

    if (shell_is_interactive) {
        if (!j->pgid) j->pgid = j->first_process->pid;
    }
}

void
remove_job (job *j)
{
    if (!first_job || !j) return;

    job **prev = &first_job;
    while (*prev && *prev != j) {
        prev = &(*prev)->next;
    }

    if (*prev) {
        *prev = j->next;
        free_job(j);
    }
}

int
job_is_stopped (job *j)
{
    for (process *p = j->first_process; p; p = p->next) {
        if (!p->stopped)
            return 0;
    }
    return 1;
}



int
job_is_completed (job *j)
{
    if (!j || !j->first_process) return 1;

    for (process *p = j->first_process; p; p = p->next) {
        //printf("Checking process: %p, completed: %d\n", (void *)p, p->completed);
        if (!p) return 1;
        if (!p->completed)
            return 0;
    }
    return 1;
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

    printf("Job Command: %s", j->command ? j->command : "(null)");
    printf("PGID: %d\n", j->pgid);
    printf("Notified: %s\n", j->notified ? "Yes" : "No");
    printf("File Descriptors: stdin=%d, stdout=%d, stderr=%d\n",
           j->stdin, j->stdout, j->stderr);

    printf("Processes in job:\n");
    printf("----\n");
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
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        job* j = first_job;
        process *p = NULL;

        // Find job and process associated with pid
        for (; j; j = j->next) {
            for (p = j->first_process; p; p = p->next) {
                if (p->pid == pid) break;
            }
            if (p) break;
        }

        if (!j || !p) continue;

        // Handle process termination
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            p->completed = 1;
            if (WIFSIGNALED(status)) {
                printf("Job [%d] terminated by signal %d\n", j->pgid, WTERMSIG(status));
            }
        }

        else if (WIFSTOPPED(status)) {
            p->stopped = 1;
            printf("Job [%d] stopped by signal %d\n", j->pgid, WSTOPSIG(status));            
        }

        else if (WIFCONTINUED(status)) {
            p->stopped = 0;
            printf("Job [%d] continued\n", j->pgid);
        }

        if (job_is_completed(j)) {
            if (j == first_job)
                first_job = j->next;
            remove_job(j);

        }

        else if (job_is_stopped(j)) {
            tcsetpgrp(shell_terminal, shell_pgid);
        }
    }
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
