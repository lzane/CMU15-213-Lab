/*
 * Name: lzane
 */

/*
 *
 * tsh - A tiny shell program. This program is a simplified shell.
 * It supports the following builtin commands:
 * - quit, quit the shell
 * - bg/fg, execute the job in background/foreground
 * - jobs, list the current jobs
 * The shell implements I/O redirection for non-builtin commands and the jobs
 * command. Also, pressing Ctrl-C kills all foreground jobs and Ctrl-Z stops all
 * foreground jobs.
 */

#include "csapp.h"
#include "tsh_helper.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * If DEBUG is defined, enable contracts and printing on dbg_printf.
 */
#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif

/* Function prototypes */
void eval(const char *cmdline);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void sigquit_handler(int sig);
void cleanup(void);
int builtin_command(struct cmdline_tokens *token);
int redirection(struct cmdline_tokens *token);
void unix_error(char *msg);
void waitfg(jid_t jobId);
void do_bgfg(struct cmdline_tokens *token);
int Sigprocmask(int how, const sigset_t *now, sigset_t *orig);

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg) {
    sio_printf("%s: %s\n", msg, strerror(errno));
    exit(1);
}
/*
 *
 * main - The main function of the shell.
 * Output shell prompt tsh> continuously
 * Read user input
 *
 */
int main(int argc, char **argv) {
    char c;
    char cmdline[MAXLINE_TSH]; // Cmdline for fgets
    bool emit_prompt = true;   // Emit prompt (default)

    // Redirect stderr to stdout (so that driver will get all output
    // on the pipe connected to stdout)
    if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
        perror("dup2 error");
        exit(1);
    }

    // Parse the command line
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h': // Prints help message
            usage();
            break;
        case 'v': // Emits additional diagnostic info
            verbose = true;
            break;
        case 'p': // Disables prompt printing
            emit_prompt = false;
            break;
        default:
            usage();
        }
    }

    // Create environment variable
    if (putenv("MY_ENV=42") < 0) {
        perror("putenv error");
        exit(1);
    }

    // Set buffering mode of stdout to line buffering.
    // This prevents lines from being printed in the wrong order.
    if (setvbuf(stdout, NULL, _IOLBF, 0) < 0) {
        perror("setvbuf error");
        exit(1);
    }

    // Initialize the job list
    init_job_list();

    // Register a function to clean up the job list on program termination.
    // The function may not run in the case of abnormal termination (e.g. when
    // using exit or terminating due to a signal handler), so in those cases,
    // we trust that the OS will clean up any remaining resources.
    if (atexit(cleanup) < 0) {
        perror("atexit error");
        exit(1);
    }

    // Install the signal handlers
    Signal(SIGINT, sigint_handler);   // Handles Ctrl-C
    Signal(SIGTSTP, sigtstp_handler); // Handles Ctrl-Z
    Signal(SIGCHLD, sigchld_handler); // Handles terminated or stopped child

    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);

    Signal(SIGQUIT, sigquit_handler);

    // Execute the shell's read/eval loop
    while (true) {
        if (emit_prompt) {
            printf("%s", prompt);

            // We must flush stdout since we are not printing a full line.
            fflush(stdout);
        }

        if ((fgets(cmdline, MAXLINE_TSH, stdin) == NULL) && ferror(stdin)) {
            perror("fgets error");
            exit(1);
        }

        if (feof(stdin)) {
            // End of file (Ctrl-D)
            printf("\n");
            return 0;
        }

        // Remove any trailing newline
        char *newline = strchr(cmdline, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        // Evaluate the command line
        eval(cmdline);
    }

    return -1; // control never reaches here
}

/*
 *
 * eval - Parse the command and execute it. If the command is a builtin cmd,
 * execute it directly. Or, create a child process to execute it.
 * If the job is fg job, wait for it finished. If the job is bg job,
 * then run it in the background.
 *
 */
void eval(const char *cmdline) {
    parseline_return parse_result;
    struct cmdline_tokens token;
    // Parse command line
    parse_result = parseline(cmdline, &token);

    if (builtin_command(&token) != 0) {
        return;
    }

    // not builtin cmd
    sigset_t blockMask, origMask, allMask;
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);
    sigfillset(&allMask);

    // block all signal to solve race condition
    // SIGCHILD or SIGINT before add_job
    Sigprocmask(SIG_BLOCK, &allMask, &origMask);

    pid_t child_pid;
    switch (child_pid = fork()) {
    case -1:
        unix_error("fork error");
        break;
    case 0:
        setpgid(0, 0);
        Sigprocmask(SIG_SETMASK, &origMask, NULL);
        execve(token.argv[0], token.argv, environ);
        printf("%s: Command not found\n", token.argv[0]);
        exit(1);
        break;
    default: {
        int state = parse_result == PARSELINE_BG ? BG : FG;
        add_job(child_pid, state, cmdline);
        jid_t jobId = job_from_pid(child_pid);
        Sigprocmask(SIG_SETMASK, &blockMask, NULL);

        if (parse_result == PARSELINE_FG) {
            waitfg(jobId);
        } else {
            printf("[%d] (%d) %s\n", jobId, child_pid, cmdline);
        }

        break;
    }
    }

    Sigprocmask(SIG_SETMASK, &origMask, NULL);

    return;
}

/*
 * builtin_command - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_command(struct cmdline_tokens *token) {
    if (strcmp(token->argv[0], "quit") == 0) {
        exit(0);
    } else if (strcmp(token->argv[0], "jobs") == 0) {
        sigset_t allMask, origMask;
        sigfillset(&allMask);
        Sigprocmask(SIG_BLOCK, &allMask, &origMask);
        // TODO: currently print to stdout
        list_jobs(1);
        Sigprocmask(SIG_SETMASK, &origMask, NULL);
        return 1;
    } else if (strcmp(token->argv[0], "fg") == 0 ||
               strcmp(token->argv[0], "bg") == 0) {
        do_bgfg(token);
        return 1;
    }

    return 0; /* not a builtin command */
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(struct cmdline_tokens *token) {
    if (token->argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", token->argv[0]);
        return;
    }
    int jid = -1;
    int pid = -1;
    sscanf(token->argv[1], "%%%d", &jid);
    sscanf(token->argv[1], "%d", &pid);

    if (jid == -1 && pid == -1) {
        printf("%s: argument must be a PID or %%jobid\n", token->argv[0]);
        return;
    }
    sigset_t allMask, origMask;
    sigfillset(&allMask);

    jid_t jobId;

    if (pid != -1) {
        Sigprocmask(SIG_BLOCK, &allMask, &origMask);
        jobId = job_from_pid(pid);
        int exist = job_exists(jobId);
        Sigprocmask(SIG_SETMASK, &origMask, NULL);
        if (!exist) {
            printf("(%d): No such process\n", pid);
            return;
        }
    } else {
        Sigprocmask(SIG_BLOCK, &allMask, &origMask);
        jobId = jid;
        int exist = job_exists(jobId);
        Sigprocmask(SIG_SETMASK, &origMask, NULL);
        if (!exist) {
            printf("%%%d: No such job\n", jid);
            return;
        }
    }

    Sigprocmask(SIG_BLOCK, &allMask, &origMask);
    pid_t pId = job_get_pid(jobId);
    Sigprocmask(SIG_SETMASK, &origMask, NULL);

    if (kill(-pId, SIGCONT) == -1) {
        unix_error("kill");
    }

    Sigprocmask(SIG_BLOCK, &allMask, &origMask);
    const char *cmdline = job_get_cmdline(jobId);

    if (strcmp(token->argv[0], "bg") == 0) {
        job_set_state(jobId, BG);
        printf("[%d] (%d) %s\n", jobId, pId, cmdline);
    } else if (strcmp(token->argv[0], "fg") == 0) {
        job_set_state(jobId, FG);
        waitfg(jobId);
    }
    Sigprocmask(SIG_SETMASK, &origMask, NULL);
    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(jid_t jobId) {
    sigset_t emptyMask, allMask, origMask;
    sigemptyset(&emptyMask);
    sigfillset(&allMask);

    while (1) {
        sigsuspend(&emptyMask);
        Sigprocmask(SIG_BLOCK, &allMask, &origMask);
        jid_t fgId = fg_job();
        Sigprocmask(SIG_SETMASK, &origMask, NULL);

        if (fgId != jobId) {
            break;
        }
    }

    return;
}

/*****************
 * Signal handlers
 *****************/

/*
 * Sigchld_handler - Invoked when catch SIGCHLD
 * It can reap all zombie children. Each child process
 * will send the SIGCHLD signals to their parent process when
 * they finish.
 */
void sigchld_handler(int sig) {
    int origErrno = errno;
    int pid, stat;
    sigset_t allMask, origMask;
    sigfillset(&allMask);

    Sigprocmask(SIG_BLOCK, &allMask, &origMask);
    while ((pid = waitpid(-1, &stat, WUNTRACED | WNOHANG)) > 0) {
        jid_t jobId = job_from_pid(pid);
        // normal exit
        if (WIFEXITED(stat)) {
            delete_job(jobId);
            continue;
        }
        // exit by signal
        if (WIFSIGNALED(stat)) {
            sio_printf("Job [%d] (%d) terminated by signal %d\n", jobId, pid,
                   WTERMSIG(stat));

            delete_job(jobId);
            continue;
        }

        if (WIFSTOPPED(stat)) {
            sio_printf("Job [%d] (%d) stopped by signal %d\n", jobId, pid,
                   WSTOPSIG(stat));
            job_set_state(jobId, ST);
            continue;
        }
    }
    Sigprocmask(SIG_SETMASK, &origMask, NULL);

    if (pid == -1 && errno != ECHILD) {
        // unix_error("waitpid");
    }

    errno = origErrno;
    return;
}

/*
 * Sigint_handler - Invoked when catch SIGINT
 * Send the SIGINT signals to fg jobs
 */
void sigint_handler(int sig) {
    int origErrno = errno;
    sigset_t allMask, origMask;
    sigfillset(&allMask);

    Sigprocmask(SIG_BLOCK, &allMask, &origMask);
    jid_t jobId = fg_job();
    pid_t pid = job_get_pid(jobId);
    Sigprocmask(SIG_SETMASK, &origMask, NULL);

    // no fg job
    if (jobId == 0) {
        errno = origErrno;
        return;
    }

    if (kill(-pid, SIGINT) == -1) {
        unix_error("kill");
    }
    errno = origErrno;
    return;
}

/*
 * Sigtstp_handler - Invoked when catch SIGTSTP
 * Send the SIGTSTP signals to fg jobs
 */
void sigtstp_handler(int sig) {
    int origErrno = errno;
    sigset_t allMask, origMask;
    sigfillset(&allMask);

    Sigprocmask(SIG_BLOCK, &allMask, &origMask);
    jid_t jobId = fg_job();
    pid_t pid = job_get_pid(jobId);
    Sigprocmask(SIG_SETMASK, &origMask, NULL);

    if (jobId == 0) {
        errno = origErrno;
        return;
    }

    if (kill(-pid, sig) == -1) {
        unix_error("kill");
    }
    errno = origErrno;
    return;
}

/*
 * cleanup - Attempt to clean up global resources when the program exits. In
 * particular, the job list must be freed at this time, since it may contain
 * leftover buffers from existing or even deleted jobs.
 */
void cleanup(void) {
    // Signals handlers need to be removed before destroying the joblist
    Signal(SIGINT, SIG_DFL);  // Handles Ctrl-C
    Signal(SIGTSTP, SIG_DFL); // Handles Ctrl-Z
    Signal(SIGCHLD, SIG_DFL); // Handles terminated or stopped child

    destroy_job_list();
}

/*****************
 * Helper functions
 *****************/
int Sigprocmask(int how, const sigset_t *now, sigset_t *orig) {
    int res = sigprocmask(how, now, orig);
    if (res == -1) {
        unix_error("Sigprocmask");
    }

    return res;
}

/*
 * Redirect I/O if specified in cmd
 * Args: struct cmdline_tokens *token
 * Return: 1 if redirect successfully, otherwise 0
 */
int redirection(struct cmdline_tokens *token) {
    return 0;
}