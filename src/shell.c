#include "shell.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE 4096
#define MAX_ARGS 256

typedef struct redir_s {
    int syntax_err;
    const char *outfile;
}   redir_t;

static int parse_line(char *line, char **argv, int max_args, redir_t *redir)
{
    int argc = 0;
    char *save = NULL;
    char *tok = strtok_r(line, " \t", &save);
    while (tok != NULL && argc < max_args - 1) {
        argv[argc++] = tok;
        tok = strtok_r(NULL, " \t", &save);
    }
    argv[argc] = NULL;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if(i == 0 || argv[i+1] == NULL)
            {
                redir->syntax_err = 1;
                redir->outfile = NULL;
                fprintf(stderr, "minish: syntax error near '>'\n");
            }
            else
            {
                redir->syntax_err = 0;
                redir->outfile = argv[i+1];
                argv[i] = NULL;
            }

            break;
        }
    }

    return argc;
}

static int run_command(char *const argv[], const char *outfile)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("minish: fork");
        return -1;
    }
    if (pid == 0) {
        if (outfile)
        {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                fprintf(stderr, "minish: %s: %s\n", outfile, strerror(errno));
                _exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(argv[0], argv);
        fprintf(stderr, "minish: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) printf("[exit %d]\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }

    return -1;
}

static int builtin_cd(int argc, char *const argv[])
{
    const char *dir;

    if (argc >= 2)  dir = argv[1];
    else dir = getenv("HOME");

    if (dir == NULL || *dir == '\0')
    {
        fprintf(stderr, "minish: cd: HOME not set\n");
        return 1;
    }

    if (chdir(dir))
    {
        fprintf(stderr, "minish: cd: %s: %s\n", dir, strerror(errno));
        return 1;
    }

    return 0;
}

int shell_loop(void)
{
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    char buf[1024];

    printf("minish v0.1 - type 'exit' or 'quit' to leave\n");

    for (;;) {
        if (getcwd(buf, sizeof buf) == NULL) {
            snprintf(buf, sizeof buf, "?");
        }

        printf("minish>%s ", buf);
        fflush(stdout);

        if (fgets(line, sizeof line, stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';

        redir_t redir = {
            .syntax_err = 0,
            .outfile = NULL
        };
        int argc = parse_line(line, argv, MAX_ARGS, &redir);
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) break;

        if (strcmp(argv[0], "cd") == 0)
        {
            builtin_cd(argc, argv);
            continue;
        }

        if (!redir.syntax_err) run_command(argv, redir.outfile);
    }

    return 0;
}
