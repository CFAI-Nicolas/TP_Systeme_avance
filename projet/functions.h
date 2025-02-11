#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "typedef.h"

// Déclaration des fonctions
void display_prompt();
void save_to_history(const char *command);
int handle_builtin_commands(char *args[]);
int run_single_command(char *cmd);
void execute_commands(char *input);

#endif
