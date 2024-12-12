#include "terminal.h"
#include <stdio.h>
#include <string.h>

void terminal_init(terminal_context_t *context) {
    context->command_count = 0;
    context->history_index = 0;
    context->authenticated = 0;
    memset(context->command_history, 0, sizeof(context->command_history));
}

void terminal_register_command(terminal_context_t *context, const char *command, const char *description, terminal_command_handler_t handler) {
    if (context->command_count < MAX_COMMANDS) {
        context->command_table[context->command_count++] = (terminal_command_t){command, description, handler};
    } else {
        printf("Errore: numero massimo di comandi raggiunto.\n");
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
            printf("Errore: comando vuoto.\n");
        } else {
            if (!terminal_is_authenticated(context) && strcmp(args[0], "LOGIN") != 0) {
                printf("Errore: autenticazione richiesta. Usa 'LOGIN <password>' per continuare.\n");
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
                printf("Errore: comando sconosciuto '%s'. Usa 'HELP'.\n", args[0]);
            }
        }
        command = strtok(NULL, ";");
    }
}

void terminal_show_history(terminal_context_t *context) {
    printf("Storico comandi:\n");
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (context->history_index + i) % HISTORY_SIZE;
        if (context->command_history[idx][0] != '\0') {
            printf(" %d: %s\n", i + 1, context->command_history[idx]);
        }
    }
}

void terminal_set_authenticated(terminal_context_t *context, int state) {
    context->authenticated = state;
}

int terminal_is_authenticated(terminal_context_t *context) {
    return context->authenticated;
}