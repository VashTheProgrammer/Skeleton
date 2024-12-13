#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

#define CMD_BUFFER_SIZE 128
#define HISTORY_SIZE 10
#define MAX_ARGS 10
#define MAX_COMMANDS 20

// codici per il VT100
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

// Dichiarazione anticipata della struttura per compatibilità
struct terminal_context_t;

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
    int enable_vt100_features; // Abilita/disabilita VT100
} terminal_context_t;

// Funzioni principali
void terminal_init(terminal_context_t *context);
void terminal_register_command(terminal_context_t *context, const char *command, const char *description, terminal_command_handler_t handler);
void terminal_execute_command(terminal_context_t *context, const char *cmd);
void terminal_show_history(terminal_context_t *context);
void terminal_set_authenticated(terminal_context_t *context, int state);
int terminal_is_authenticated(terminal_context_t *context);
void terminal_show_prompt(terminal_context_t *context);
void terminal_print_message(const char *message, const char *color, terminal_context_t *context);

#endif // TERMINAL_H
