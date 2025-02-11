#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "functions.h"

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_SIZE];

    if (argc == 3 && strcmp(argv[1], "-c") == 0) {
        strcpy(input, argv[2]);
        execute_commands(input);
        return 0;
    }

    while (1) {
        display_prompt();
        
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            perror("fgets error");
            exit(EXIT_FAILURE);
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) > 0) {  
            save_to_history(input);
        }

        if (strcmp(input, "exit") == 0) {
            printf("Exiting my_shell...\n");
            break;
        }

        execute_commands(input);
    }

    return 0;
}

