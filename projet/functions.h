#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "typedef.h"

// Déclaration des fonctions
void display_prompt();
int handle_builtin_commands(char *args[]);
void save_to_history(const char *command);
void handle_redirection(char **args);
int handle_pipe(char *cmd);
int handle_builtin_commands(char *args[]);
int run_single_command(char *cmd);
void execute_commands(char *input);

#endif
