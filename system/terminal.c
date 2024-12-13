#include "terminal.h"
#include <stdio.h>
#include <string.h>

void terminal_init(terminal_context_t *context) {
    context->command_count = 0;
    context->history_index = 0;
    context->authenticated = 0;
    context->enable_vt100_features = 0; // Di default, VT100 è disabilitato
    memset(context->command_history, 0, sizeof(context->command_history));
}

void terminal_register_command(terminal_context_t *context, const char *command, const char *description, terminal_command_handler_t handler) {
    if (context->command_count < MAX_COMMANDS) {
        context->command_table[context->command_count++] = (terminal_command_t){command, description, handler};
    } else {
        terminal_print_message("Errore: numero massimo di comandi raggiunto.\n", COLOR_RED, context);
    }
}

void terminal_execute_command(terminal_context_t *context, const char *cmd) {
    char *command = strtok((char *)cmd, ";");
    while (command) {
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
            terminal_print_message("Errore: comando vuoto.\n", COLOR_RED, context);
        } else {
            if (!terminal_is_authenticated(context) && strcmp(args[0], "LOGIN") != 0) {
                terminal_print_message("Errore: autenticazione richiesta. Usa 'LOGIN <password>' per continuare.\n", COLOR_RED, context);
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
                snprintf(error_message, CMD_BUFFER_SIZE, "Errore: comando sconosciuto '%s'. Usa 'HELP'.\n", args[0]);
                terminal_print_message(error_message, COLOR_RED, context);
            }
        }

        // Salva il comando nello storico
        strncpy(context->command_history[context->history_index], cmd, CMD_BUFFER_SIZE - 1);
        context->command_history[context->history_index][CMD_BUFFER_SIZE - 1] = '\0';
        context->history_index = (context->history_index + 1) % HISTORY_SIZE;

        command = strtok(NULL, ";");
    }

    // Mostra il prompt solo se il comando non è LOGIN
    if (terminal_is_authenticated(context) && strcmp(cmd, "LOGIN") != 0) {
        terminal_show_prompt(context);
    }
}

void terminal_show_history(terminal_context_t *context) {
    terminal_print_message("Storico comandi:\n", COLOR_BLUE, context);
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (context->history_index - i - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        if (context->command_history[idx][0] != '\0') {
            char history_message[CMD_BUFFER_SIZE];
            snprintf(history_message, CMD_BUFFER_SIZE, " %d: %s\n", HISTORY_SIZE - i, context->command_history[idx]);
            terminal_print_message(history_message, COLOR_BLUE, context);
        }
    }
}

void terminal_set_authenticated(terminal_context_t *context, int state) {
    context->authenticated = state;
    if (state) {
        terminal_print_message("Autenticazione riuscita.\n", COLOR_GREEN, context);
    } else {
        terminal_print_message("Disconnessione avvenuta.\n", COLOR_BLUE, context);
    }
}

int terminal_is_authenticated(terminal_context_t *context) {
    return context->authenticated;
}

void terminal_show_prompt(terminal_context_t *context) {
    if (context->enable_vt100_features) {
        // Prompt in verde
        terminal_print_message("skeleton> ", COLOR_GREEN, context);
    } else {
        printf("skeleton> "); // Prompt normale
    }
}

void terminal_print_message(const char *message, const char *color, terminal_context_t *context) {
    if (context->enable_vt100_features && color) {
        printf("%s%s%s", color, message, COLOR_RESET);
    } else {
        printf("%s", message);
    }
}
