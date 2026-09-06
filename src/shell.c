#include "shell.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 4096
#define MAX_ARGS 256

static int last_status = 0;

static const char *get_var(const char *name, size_t len)
{
    static char buf[64];

    if (len == 1 && name[0] == '?') {
        snprintf(buf, sizeof buf, "%d", last_status);
        return buf;
    }
    if (len == 1 && name[0] == '$') {
        snprintf(buf, sizeof buf, "%ld", (long)getpid());
        return buf;
    }
    if (len == 0 || len >= sizeof buf) {
        return "";
    }
    memcpy(buf, name, len);
    buf[len] = '\0';
    const char *v = getenv(buf);
    return v ? v : "";
}

static size_t expand(const char *src, char *dst, size_t dst_size)
{
    size_t di = 0;
    size_t si = 0;
    while (src[si] != '\0' && di + 1 < dst_size) {
        if (src[si] != '$') {
            dst[di++] = src[si++];
            continue;
        }
        if (src[si + 1] == '\0') {
            dst[di++] = src[si++];
            break;
        }
        size_t name_len = 0;
        char next = src[si + 1];
        if (next == '$' || next == '?') {
            name_len = 1;
        } else if (isalnum((unsigned char)next) || next == '_') {
            while (src[si + 1 + name_len] != '\0' &&
                   (isalnum((unsigned char)src[si + 1 + name_len]) ||
                    src[si + 1 + name_len] == '_')) {
                name_len++;
            }
        }
        if (name_len == 0) {
            dst[di++] = src[si++];
            continue;
        }
        const char *val = get_var(src + si + 1, name_len);
        size_t vlen = strlen(val);
        if (di + vlen + 1 >= dst_size) break;
        memcpy(dst + di, val, vlen);
        di += vlen;
        si += 1 + name_len;
    }
    dst[di] = '\0';
    return di;
}

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
        return 1;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "minish: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return 1;
}

static int builtin_cd(int argc, char *const argv[])
{
    const char *dir = (argc > 1) ? argv[1] : getenv("HOME");
    if (dir == NULL || *dir == '\0') {
        fprintf(stderr, "minish: cd: HOME not set\n");
        return 1;
    }
    if (chdir(dir) != 0) {
        fprintf(stderr, "minish: cd: %s: %s\n", dir, strerror(errno));
        return 1;
    }
    return 0;
}

static int builtin_exit(int argc, char *const argv[])
{
    int code = (argc > 1) ? atoi(argv[1]) : 0;
    exit(code);
    return 0;
}

static int builtin_echo(int argc, char *const argv[])
{
    for (int i = 1; i < argc; i++) {
        if (i > 1) putchar(' ');
        fputs(argv[i], stdout);
    }
    putchar('\n');
    return 0;
}

static int builtin_pwd(int argc, char *const argv[])
{
    (void)argc;
    (void)argv;
    char cwd[4096];
    if (getcwd(cwd, sizeof cwd) == NULL) {
        perror("minish: pwd");
        return 1;
    }
    puts(cwd);
    return 0;
}

struct builtin {
    const char *name;
    int (*fn)(int argc, char *const argv[]);
};

static const struct builtin builtins[] = {
    {"cd",   builtin_cd},
    {"exit", builtin_exit},
    {"echo", builtin_echo},
    {"pwd",  builtin_pwd},
    {NULL, NULL},
};

static int run_builtin(int argc, char *const argv[])
{
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(argv[0], builtins[i].name) == 0) {
            return builtins[i].fn(argc, argv);
        }
    }
    return -1;
}

int shell_loop(void)
{
    char line[MAX_LINE];
    char expanded[MAX_LINE];
    char *argv[MAX_ARGS];

    for (;;) {
        printf("minish> ");
        fflush(stdout);

        if (fgets(line, sizeof line, stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';

        expand(line, expanded, sizeof expanded);

        int argc = parse_line(expanded, argv, MAX_ARGS);
        if (argc == 0) continue;

        int rc = run_builtin(argc, argv);
        if (rc >= 0) {
            last_status = rc;
        } else {
            last_status = run_command(argv);
        }
    }
    return last_status;
}
