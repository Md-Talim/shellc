#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
    // Flush after every printf
    setbuf(stdout, NULL);

    while (1) {
        printf("$ ");

        char input[256];
        fgets(input, 256, stdin);
        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "exit") == 0) {
            break;
        }

        if (strncmp(input, "echo ", 5) == 0) {
            printf("%s\n", input + 5);
        } else if (strncmp(input, "type ", 5) == 0) {
            char *command = input + 5;
            if ((strcmp(command, "echo") == 0) ||
                (strcmp(command, "exit") == 0) ||
                (strcmp(command, "type") == 0)) {
                printf("%s is a shell builtin\n", command);
            } else {
                char *executable = find_executable(command);
                if (executable) {
                    printf("%s is %s\n", command, executable);
                    free(executable);
                    executable = NULL;
                } else {
                    printf("%s: not found\n", command);
                }
            }
        } else {
            printf("%s: command not found\n", input);
        }
    }

    return 0;
}
