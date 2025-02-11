#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>  // Pour open(), O_APPEND, O_WRONLY

#include "functions.h"

// Fonction pour afficher le prompt
void display_prompt() {
    printf("my_shell> ");
}

// Fonction pour ajouter une commande à l’historique
void save_to_history(const char *command) {
    int fd = open(HISTORY_FILE, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening history file");
        return;
    }
    write(fd, command, strlen(command));
    write(fd, "\n", 1);
    close(fd);
}

// Gestion des commandes internes
int handle_builtin_commands(char *args[]) {
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting my_shell...\n");
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        }
        return 1;
    }
    else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
        }
        return 1;
    }
    else if (strcmp(args[0], "history") == 0) {
        FILE *file = fopen(HISTORY_FILE, "r");
        if (file == NULL) {
            perror("Error opening history file");
            return 1;
        }

        char line[MAX_INPUT_SIZE];
        int count = 1;
        while (fgets(line, sizeof(line), file)) {
            printf("%d: %s", count++, line);
        }
        fclose(file);
        return 1;
    }
    return 0;
}

// Exécuter une seule commande
int run_single_command(char *cmd) {
    char *args[MAX_ARG_SIZE];
    char *token;
    int i = 0;

    token = strtok(cmd, " ");
    while (token != NULL && i < MAX_ARG_SIZE - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL) {
        return 1;
    }

    if (handle_builtin_commands(args)) {
        return 0;
    }

    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("Command not found");
        }
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }

    return 0;
}

// Fonction pour exécuter plusieurs commandes
void execute_commands(char *input) {
    char *cmd;
    int last_status = 0;

    char *rest = input;
    while ((cmd = strsep(&rest, ";")) != NULL) {
        char *subcmd;
        char *subrest = cmd;

        while ((subcmd = strsep(&subrest, "&&")) != NULL) {
            if (last_status == 0) {
                last_status = run_single_command(subcmd);
            }
        }

        while ((subcmd = strsep(&subrest, "||")) != NULL) {
            if (last_status != 0) {
                last_status = run_single_command(subcmd);
            }
        }
    }
}

