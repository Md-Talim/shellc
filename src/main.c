#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT 4096
#define MAX_ARGS 256

static const char *BUILTINS[] = {"echo", "exit", "pwd", "type", NULL};

// Splits input in-place on spaces, filling argv with pointers into input.
// Returns the number of arguments (argv is NULL-terminated at argv[argc]).
// returns 0 for an empty/whitespace only line
int parse_input(char *input, char **argv, int max_args) {
    int argc = 0;
    int i = 0;

    while (input[i] != '\0') {
        while (input[i] == ' ') {
            i++;
        }
        if (input[i] == '\0') {
            break;
        }
        if (argc >= max_args - 1) {
            break;
        }

        argv[argc++] = &input[i];

        while (input[i] != ' ' && input[i] != '\0') {
            i++;
        }
        if (input[i] == ' ') {
            input[i] = '\0';
            i++;
        }
    }
    argv[argc] = NULL;

    return argc;
}

char *find_executable(char *command) {
    char *path = getenv("PATH");
    if (!path) {
        return NULL;
    }

    int start = 0;
    int path_len = strlen(path);

    for (int c = 0; c <= path_len; c++) {
        if (path[c] != ':' && path[c] != '\0') {
            continue;
        }

        int cmd_len = strlen(command);
        int dir_len = c - start;

        // dir_len + '/' + cmd_len
        int full_len = dir_len + 1 + cmd_len;

        char *executable = malloc(full_len + 1); // extra 1 for the '\0'
        if (executable == NULL) {
            printf("malloc failed\n");
            return NULL;
        }

        strncpy(executable, path + start, dir_len);
        executable[dir_len] = '/';
        strncpy(executable + dir_len + 1, command, cmd_len);

        executable[full_len] = '\0';

        if (access(executable, X_OK) == 0) {
            return executable;
        }

        free(executable);
        executable = NULL;
        start = c + 1;
    }

    return NULL;
}

int is_builtin(const char *name) {
    for (int i = 0; BUILTINS[i]; i++) {
        if (strcmp(name, BUILTINS[i]) == 0)
            return 1;
    }
    return 0;
}

int run_builtin(char **args, int argc) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; i < argc; i++) {
            printf("%s%s", args[i], i == argc - 1 ? "\n" : " ");
        }
        if (argc == 1) {
            printf("\n");
        }
        return 1;
    }

    if (strcmp(args[0], "pwd") == 0) {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL) {
            fprintf(stderr, "getcwd failed\n");
        } else {
            printf("%s\n", cwd);
            free(cwd);
        }
        return 1;
    }

    if (strcmp(args[0], "type") == 0) {
        if (argc < 2) {
            fprintf(stderr, "type: usage: type [name ...]\n");
            return 1;
        }
        for (int i = 1; i < argc; i++) {
            if (is_builtin(args[i])) {
                printf("%s is a shell builtin\n", args[i]);
            } else {
                char *executable = find_executable(args[i]);
                if (executable) {
                    printf("%s is %s\n", args[i], executable);
                    free(executable);
                } else {
                    printf("%s: not found\n", args[i]);
                }
            }
        }
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // Flush after every printf
    setbuf(stdout, NULL);

    while (1) {
        printf("$ ");

        char input[MAX_INPUT];
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break; // ctrl-D / EOF
        }
        input[strcspn(input, "\n")] = '\0';

        char *args[MAX_ARGS];
        int nargs = parse_input(input, args, MAX_ARGS);
        if (nargs == 0) {
            continue; // empty line
        }

        if (run_builtin(args, nargs)) {
            continue;
        }

        char *executable = find_executable(args[0]);
        if (executable == NULL) {
            printf("%s: command not found\n", args[0]);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) { // failed
            perror("fork failed");
            free(executable);
            continue;
        }
        if (pid == 0) {
            execv(executable, args);
            perror("execv failed");
            free(executable);
            _exit(1);
        }

        waitpid(pid, NULL, 0);
        free(executable);
    }

    return 0;
}
