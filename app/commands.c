#include "commands.h"
#include <stdio.h>
#include <string.h>
//#include "hardware/flash.h"

#define FLASH_TARGET_OFFSET 0x100000
#define CONFIG_SIZE 256

static char config[CONFIG_SIZE];

void cmd_help(terminal_context_t *context, size_t argc, char **argv) {
    printf("Comandi disponibili:\n");
    for (size_t i = 0; i < context->command_count; i++) {
        printf(" - %s: %s\n", context->command_table[i].command, context->command_table[i].description);
    }
}

void cmd_login(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        printf("Errore: specificare una password.\n");
        return;
    }
    if (strcmp(argv[1], "password123") == 0) {
        terminal_set_authenticated(context, 1);
        printf("Accesso eseguito con successo.\n");
    } else {
        printf("Errore: password errata.\n");
    }
}

void cmd_logout(terminal_context_t *context, size_t argc, char **argv) {
    terminal_set_authenticated(context, 0);
    printf("Logout eseguito. Autenticazione richiesta per ulteriori comandi.\n");
}

/*
void cmd_save(terminal_context_t *context, size_t argc, char **argv) {
    flash_range_erase(FLASH_TARGET_OFFSET, CONFIG_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, (uint8_t *)config, CONFIG_SIZE);
    printf("Configurazione salvata.\n");
}

void cmd_load(terminal_context_t *context, size_t argc, char **argv) {
    memcpy(config, (const void *)(XIP_BASE + FLASH_TARGET_OFFSET), CONFIG_SIZE);
    printf("Configurazione caricata.\n");
}

void cmd_set_config(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 3) {
        printf("Errore: specificare un parametro e un valore.\n");
        return;
    }
    snprintf(config, CONFIG_SIZE, "%s=%s", argv[1], argv[2]);
    printf("Configurazione aggiornata: %s\n", config);
}
*/

void cmd_history(terminal_context_t *context, size_t argc, char **argv) {
    terminal_show_history(context);
}

void init_commands(terminal_context_t *context) {
    terminal_register_command(context, "HELP", "Mostra la lista dei comandi", cmd_help);
    terminal_register_command(context, "HISTORY", "Mostra lo storico dei comandi", cmd_history);
    terminal_register_command(context, "LOGIN", "Effettua il login", cmd_login);
    terminal_register_command(context, "LOGOUT", "Disconnette l'utente", cmd_logout);
    //terminal_register_command(context, "SAVE", "Salva la configurazione", cmd_save);
    //terminal_register_command(context, "LOAD", "Carica la configurazione", cmd_load);
    //terminal_register_command(context, "SET", "Aggiorna la configurazione", cmd_set_config);
}
