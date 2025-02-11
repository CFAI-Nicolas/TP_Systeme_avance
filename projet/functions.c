#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "functions.h"

// Fonction pour afficher le prompt
void display_prompt() {
    printf("my_shell> ");
}

// Implémentation des commandes internes de linux
int handle_builtin_commands(char *args[]) {
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting my_shell...\n");
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        }
        return 1;
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
        }
        return 1;
    }
    return 0;
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

// Gestion des redirections
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("Failed to open file for output redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], ">>") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                perror("Failed to open file for append output redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd == -1) {
                perror("Failed to open file for input redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<<") == 0) {
    char buffer[1024];
    int fd[2];
    pipe(fd);

    if (fork() == 0) {
        close(fd[0]);
        printf("heredoc> ");
        while (fgets(buffer, sizeof(buffer), stdin)) {
            if (strncmp(buffer, args[i + 1], strlen(args[i + 1])) == 0) {
                break; 
            }
            write(fd[1], buffer, strlen(buffer));
            printf("heredoc> ");
        }
        close(fd[1]);
        exit(0);
    }

    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    args[i] = NULL;
}

    }
}

// Pour commande avec des pipes
int handle_pipes(char *cmd) {
    char *commands[MAX_ARG_SIZE];
    int pipefd[2], input_fd = 0;
    pid_t pid;

    int num_cmds = 0;
    char *token = strtok(cmd, "|");
    while (token != NULL && num_cmds < MAX_ARG_SIZE - 1) {
        commands[num_cmds++] = token;
        token = strtok(NULL, "|");
    }
    commands[num_cmds] = NULL;

    for (int i = 0; i < num_cmds; i++) {
        pipe(pipefd);

        pid = fork();
        if (pid == -1) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) {
            dup2(input_fd, STDIN_FILENO);
            if (i < num_cmds - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);
            close(pipefd[1]);

            char *args[MAX_ARG_SIZE];
            char *arg_token = strtok(commands[i], " ");
            int j = 0;
            while (arg_token != NULL && j < MAX_ARG_SIZE - 1) {
                args[j++] = arg_token;
                arg_token = strtok(NULL, " ");
            }
            args[j] = NULL;

            execvp(args[0], args);
            perror("Command not found");
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, NULL, 0);
            close(pipefd[1]);
            input_fd = pipefd[0];
        }
    }

    return 0;
}


// Exécution d'une commande
int run_single_command(char *cmd) {
    if (strchr(cmd, '|') != NULL) {
        return handle_pipes(cmd);
    }

    char *args[MAX_ARG_SIZE];
    char *token;
    int i = 0;
    int background = 0;

    if (cmd[strlen(cmd) - 1] == '&') {
        background = 1;
        cmd[strlen(cmd) - 1] = '\0';
    }

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
        handle_redirection(args);
        execvp(args[0], args);
        perror("Command not found");
        exit(EXIT_FAILURE);
    } else {
        if (!background) {
            waitpid(pid, NULL, 0);
        } else {
            printf("[Processus en arrière-plan lancé : PID %d]\n", pid);
        }
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

