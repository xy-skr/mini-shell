# mini-shell

A tiny Unix shell written in C. Learning project to grow from beginner to
contributing member of the open source community.

## Goals

- Understand how a shell works from the inside (parsing, process control,
  file descriptors, signals).
- Practice real C engineering: build systems, header discipline, testing,
  debugging, version control.
- Build a portfolio piece that shows the journey.

## Features

| Version | Status | Description |
|---------|--------|-------------|
| v0      | done   | REPL, fork/exec/wait, `exit` builtin |
| v1      | todo   | Builtins: `cd`, `exit`, `echo`, environment variables |
| v2      | todo   | Pipes (`\|`) and redirections (`>`, `<`) |
| v3      | todo   | Signal handling, robust error reporting |
| v4      | todo   | History, line editing, job control |

## Build

Requires a C11 compiler and GNU make. Tested with gcc 14+ and clang 17+.

```sh
make            # optimized build
make debug      # ASan + UBSan build, no optimization
make run        # build and launch
make clean      # remove build artifacts
```

## Usage

```sh
$ make run
minish> ls
Makefile  src  README.md
minish> echo hello world
hello world
minish> exit
```

## Project Layout

```
mini-shell/
  src/           C source files
  Makefile       build rules
  LICENSE        MIT
  README.md      you are here
```

## License

MIT. See [LICENSE](LICENSE).
