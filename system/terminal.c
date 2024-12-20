#include "terminal.h"
#include <stdio.h>
#include <string.h>

// Initializes the terminal context
// Resets all fields and prepares the context for use.
void terminal_init(terminal_context_t *context) {
    context->command_count = 0;
    context->history_index = 0;
    context->authenticated = 0;
    context->enable_vt100_features = 0; // VT100 features are disabled by default
    memset(context->command_history, 0, sizeof(context->command_history));
}

// Registers a command in the terminal context
// Adds a command, description, and handler function to the command table.
void terminal_register_command(terminal_context_t *context, const char *command, const char *description, terminal_command_handler_t handler) {
    if (context->command_count < MAX_COMMANDS) {
        context->command_table[context->command_count++] = (terminal_command_t){command, description, handler};
    } else {
        terminal_print_message("[SYSTEM][ERROR] Maximum number of commands reached.\n", COLOR_RED, context);
    }
}

// Executes a given command string
// Parses the command and its arguments, then invokes the appropriate handler.
void terminal_execute_command(terminal_context_t *context, const char *cmd) {
    char command_buffer[CMD_BUFFER_SIZE];
    size_t cmd_len = strlen(cmd);

    // Duplicate the string to avoid modifying the original
    strncpy(command_buffer, cmd, CMD_BUFFER_SIZE - 1);
    command_buffer[CMD_BUFFER_SIZE - 1] = '\0';

    char *command = command_buffer;
    char *next_command = NULL;

    do {
        // Find the next delimiter ";"
        next_command = strchr(command, ';');
        if (next_command) {
            *next_command = '\0'; // Terminate the current command
            next_command++;       // Move to the next command
        }

        // Trim leading and trailing spaces
        while (*command == ' ') command++;
        char *end = command + strlen(command) - 1;
        while (end > command && *end == ' ') {
            *end = '\0';
            end--;
        }

        // Process the command if it is not empty
        if (strlen(command) > 0) {
            char cmd_copy[CMD_BUFFER_SIZE];
            strncpy(cmd_copy, command, CMD_BUFFER_SIZE);

            char *args[MAX_ARGS];
            size_t argc = 0;
            char *token = strtok(cmd_copy, " ");
            while (token && argc < MAX_ARGS) {
                args[argc++] = token;
                token = strtok(NULL, " ");
            }

            if (argc == 0) {
                terminal_print_message("[SYSTEM][ERROR] Empty command.\n", COLOR_RED, context);
            } else {
                if (!terminal_is_authenticated(context) && strcmp(args[0], "LOGIN") != 0) {
                    terminal_print_message("[SYSTEM][ERROR] Authentication required. Use 'LOGIN <password>' to proceed.\n", COLOR_RED, context);
                    return;
                }

                int found = 0;
                for (size_t i = 0; i < context->command_count; i++) {
                    if (strcmp(args[0], context->command_table[i].command) == 0) {
                        context->command_table[i].handler(context, argc, args);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    char error_message[CMD_BUFFER_SIZE];
                    snprintf(error_message, CMD_BUFFER_SIZE, "[SYSTEM][ERROR] Unknown command '%s'. Use 'HELP'.\n", args[0]);
                    terminal_print_message(error_message, COLOR_RED, context);
                }
            }

            // Add the current command to the history
            strncpy(context->command_history[context->history_index], command, CMD_BUFFER_SIZE - 1);
            context->command_history[context->history_index][CMD_BUFFER_SIZE - 1] = '\0';
            context->history_index = (context->history_index + 1) % HISTORY_SIZE;
        }

        command = next_command; // Move to the next command

    } while (command != NULL);

    // Show the prompt if the command is not LOGIN
    if (terminal_is_authenticated(context)) {
        terminal_show_prompt(context);
    }
}

// Displays the command history
// Prints a numbered list of previous commands.
void terminal_show_history(terminal_context_t *context) {
    terminal_print_message("[SYSTEM] Command history:\n", COLOR_BLUE, context);
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (context->history_index - i - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        if (context->command_history[idx][0] != '\0') {
            char history_message[CMD_BUFFER_SIZE];
            snprintf(history_message, CMD_BUFFER_SIZE, " %d: %s\n", HISTORY_SIZE - i, context->command_history[idx]);
            terminal_print_message(history_message, COLOR_BLUE, context);
        }
    }
}

// Sets the authentication state of the terminal
void terminal_set_authenticated(terminal_context_t *context, int state) {
    context->authenticated = state;
    if (state) {
        terminal_print_message("[SYSTEM] Authentication successful.\n", COLOR_GREEN, context);
    } else {
        terminal_print_message("[SYSTEM] Disconnected.\n", COLOR_BLUE, context);
    }
}

// Checks if the terminal is authenticated
int terminal_is_authenticated(terminal_context_t *context) {
    return context->authenticated;
}

// Displays the terminal prompt
// Shows a VT100-style colored prompt if enabled.
void terminal_show_prompt(terminal_context_t *context) {
    if (context->enable_vt100_features) {
        terminal_print_message("skeleton> ", COLOR_GREEN, context);
    } else {
        printf("skeleton> "); // Standard prompt
    }
}

// Prints a message to the terminal
// Uses VT100 colors if enabled.
void terminal_print_message(const char *message, const char *color, terminal_context_t *context) {
    if (context->enable_vt100_features && color) {
        printf("%s%s%s", color, message, COLOR_RESET);
    } else {
        printf("%s", message);
    }
}
