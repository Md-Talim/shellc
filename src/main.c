#include <stdio.h>
#include <string.h>

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
            else
                printf("%s: not found\n", command);
        } else {
            printf("%s: command not found\n", input);
        }
    }

    return 0;
}
