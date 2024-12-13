#include "commands.h"
#include <stdio.h>
#include <string.h>

void cmd_help(terminal_context_t *context, size_t argc, char **argv) {
    terminal_print_message("Comandi disponibili:\n", COLOR_BLUE, context);
    for (size_t i = 0; i < context->command_count; i++) {
        char help_message[CMD_BUFFER_SIZE];
        snprintf(help_message, CMD_BUFFER_SIZE, " - %s: %s\n", context->command_table[i].command, context->command_table[i].description);
        terminal_print_message(help_message, COLOR_BLUE, context);
    }
}

void cmd_login(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        terminal_print_message("Errore: specificare una password.\n", COLOR_RED, context);
        return;
    }
    if (strcmp(argv[1], "password123") == 0) {
        terminal_set_authenticated(context, 1);
    } else {
        terminal_print_message("Errore: password errata.\n", COLOR_RED, context);
    }
}

void cmd_logout(terminal_context_t *context, size_t argc, char **argv) {
    terminal_set_authenticated(context, 0);
}

void cmd_history(terminal_context_t *context, size_t argc, char **argv) {
    terminal_show_history(context);
}

void cmd_enable_vt100(terminal_context_t *context, size_t argc, char **argv) {
    context->enable_vt100_features = 1;
    terminal_print_message("VT100 abilitato. Prompt colorato e cursore lampeggiante attivati.\n", COLOR_GREEN, context);
}

void cmd_disable_vt100(terminal_context_t *context, size_t argc, char **argv) {
    context->enable_vt100_features = 0;
    terminal_print_message("VT100 disabilitato. Prompt normale ripristinato.\n", COLOR_BLUE, context);
}

void init_commands(terminal_context_t *context) {
    terminal_register_command(context, "HELP", "Mostra la lista dei comandi", cmd_help);
    terminal_register_command(context, "HISTORY", "Mostra lo storico dei comandi", cmd_history);
    terminal_register_command(context, "LOGIN", "Effettua il login", cmd_login);
    terminal_register_command(context, "LOGOUT", "Disconnette l'utente", cmd_logout);
    terminal_register_command(context, "ENABLE_VT100", "Abilita funzionalità VT100", cmd_enable_vt100);
    terminal_register_command(context, "DISABLE_VT100", "Disabilita funzionalità VT100", cmd_disable_vt100);
}
