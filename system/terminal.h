#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

#define CMD_BUFFER_SIZE 128
#define HISTORY_SIZE 10
#define MAX_ARGS 10
#define MAX_COMMANDS 20

typedef void (*terminal_command_handler_t)(struct terminal_context_t *context, size_t argc, char **argv);

typedef struct {
    const char *command;
    const char *description;
    terminal_command_handler_t handler;
} terminal_command_t;

typedef struct terminal_context_t {
    terminal_command_t command_table[MAX_COMMANDS];
    size_t command_count;
    char command_history[HISTORY_SIZE][CMD_BUFFER_SIZE];
    int history_index;
    int authenticated;
} terminal_context_t;

// Funzioni principali
void terminal_init(terminal_context_t *context);
void terminal_register_command(terminal_context_t *context, const char *command, const char *description, terminal_command_handler_t handler);
void terminal_execute_command(terminal_context_t *context, const char *cmd);
void terminal_show_history(terminal_context_t *context);
void terminal_set_authenticated(terminal_context_t *context, int state);
int terminal_is_authenticated(terminal_context_t *context);

#endif // TERMINAL_H