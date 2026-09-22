#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
                (strcmp(command, "type") == 0))
                printf("%s is a shell builtin\n", command);
            else {
                char *path = getenv("PATH");

                int start = 0;
                int found = 0;

                for (int c = 0; c < strlen(path); c++) {
                    if (path[c] == ':') {
                        int cmd_len = strlen(command);
                        int dir_len = c - start;
                        int full_len = dir_len + cmd_len + 1; // add 1 for '\0'

                        char directory[full_len];

                        strncpy(directory, path + start, dir_len);
                        directory[dir_len] = '/';
                        strncpy(directory + dir_len + 1, command, cmd_len);

                        directory[full_len] = '\0';

                        if (access(directory, X_OK) == 0) {
                            printf("%s is %s\n", command, directory);
                            found = 1;
                            break;
                        }

                        start = c + 1;
                    }
                }

                if (!found)
                    printf("%s: not found\n", command);
            }
        } else {
            printf("%s: command not found\n", input);
        }
    }

    return 0;
}
