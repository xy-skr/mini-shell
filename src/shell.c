#include "shell.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 4096
#define MAX_ARGS 256

static int parse_line(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *save = NULL;
    char *tok = strtok_r(line, " \t", &save);
    while (tok != NULL && argc < max_args - 1) {
        argv[argc++] = tok;
        tok = strtok_r(NULL, " \t", &save);
    }
    argv[argc] = NULL;
    return argc;
}

static int run_command(char *const argv[])
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("minish: fork");
        return -1;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "minish: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[exit %d]\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    return -1;
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

        int argc = parse_line(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) break;

        run_command(argv);
    }
    return 0;
}
