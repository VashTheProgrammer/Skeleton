#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

// Buffer and history sizes
#define CMD_BUFFER_SIZE 128
#define HISTORY_SIZE 15
#define MAX_ARGS 10
#define MAX_COMMANDS 20

// VT100 color codes
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

// Forward declaration of the terminal context structure for compatibility
struct terminal_context_t;

// Function pointer type for terminal commands
typedef void (*terminal_command_handler_t)(struct terminal_context_t *context, size_t argc, char **argv);

// Command structure
typedef struct {
    const char *command;               // Command keyword
    const char *description;           // Description of the command
    terminal_command_handler_t handler; // Function handler for the command
} terminal_command_t;

// Terminal context structure
typedef struct terminal_context_t {
    terminal_command_t command_table[MAX_COMMANDS]; // Table of registered commands
    size_t command_count;                           // Number of registered commands
    char command_history[HISTORY_SIZE][CMD_BUFFER_SIZE]; // Command history buffer
    int history_index;                              // Current index in the history buffer
    int authenticated;                              // Authentication state
    int enable_vt100_features;                     // Flag to enable/disable VT100 features
} terminal_context_t;

// Terminal API
void terminal_init(terminal_context_t *context);
void terminal_register_command(terminal_context_t *context, const char *command, const char *description, terminal_command_handler_t handler);
void terminal_execute_command(terminal_context_t *context, const char *cmd);
void terminal_show_history(terminal_context_t *context);
void terminal_set_authenticated(terminal_context_t *context, int state);
int terminal_is_authenticated(terminal_context_t *context);
void terminal_show_prompt(terminal_context_t *context);
void terminal_print_message(const char *message, const char *color, terminal_context_t *context);

#endif // TERMINAL_H
