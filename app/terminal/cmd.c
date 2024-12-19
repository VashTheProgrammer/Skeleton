#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/watchdog.h"
#include "terminal/cmd.h"

#include "scheduler.h"
#include "debug.h"
#include "config.h"
#include "flash.h"

// SECRET
#define PWD "1234" 

void cmd_help(terminal_context_t *context, size_t argc, char **argv) {
    terminal_print_message("[SYSTEM] Comandi disponibili:\n", COLOR_BLUE, context);
    for (size_t i = 0; i < context->command_count; i++) {
        char help_message[CMD_BUFFER_SIZE];
        snprintf(help_message, CMD_BUFFER_SIZE, " - %s: %s\n", context->command_table[i].command, context->command_table[i].description);
        terminal_print_message(help_message, COLOR_BLUE, context);
    }
}

void cmd_login(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        terminal_print_message("[SYSTEM] Specificare una password.\n", COLOR_RED, context);
        return;
    }
    if (strcmp(argv[1], PWD) == 0) {
        terminal_set_authenticated(context, 1);
    } else {
        terminal_print_message("[SYSTEM][ERROR] Password errata.\n", COLOR_RED, context);
    }
}

void cmd_logout(terminal_context_t *context, size_t argc, char **argv) {
    terminal_set_authenticated(context, 0);
}

void cmd_history(terminal_context_t *context, size_t argc, char **argv) {
    terminal_show_history(context);
}

void cmd_reboot(terminal_context_t *context, size_t argc, char **argv) {
    terminal_print_message("[SYSTEM] Riavvio in corso...\n", COLOR_GREEN, context);
    watchdog_reboot(0, 0, 0);
}

void cmd_tasks(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        terminal_print_message("[SYSTEM][ERROR] Specificare un sottocomando (PRIO, HOLD, RESUME).\n", COLOR_RED, context);
        return;
    }

    if (strcmp(argv[1], "PS") == 0) {
        scheduler_print_task_list();
    } else if (strcmp(argv[1], "PRIO") == 0) {
        if (argc < 4) {
            terminal_print_message("[SYSTEM][ERROR] Specificare ID del task e nuova priorità.\n", COLOR_RED, context);
            return;
        }
        int task_id = atoi(argv[2]);
        int priority = atoi(argv[3]);
        if (scheduler_set_task_priority(task_id, priority) == SCHED_ERR_OK) {
            terminal_print_message("[SYSTEM] Priorità aggiornata.\n", COLOR_GREEN, context);
        } else {
            terminal_print_message("[SYSTEM][ERROR] ID task non valido.\n", COLOR_RED, context);
        }
    } else if (strcmp(argv[1], "HOLD") == 0) {
        if (argc < 3) {
            terminal_print_message("[SYSTEM][ERROR] specificare ID del task.\n", COLOR_RED, context);
            return;
        }
        int task_id = atoi(argv[2]);
        if (scheduler_pause_task(task_id) == SCHED_ERR_OK) {
            terminal_print_message("\n[SYSTEM] Task paused.\n", COLOR_GREEN, context);
        } else {
            terminal_print_message("[SYSTEM][ERROR] ID task non valido.\n", COLOR_RED, context);
        }
    } else if (strcmp(argv[1], "RUN") == 0) {
        if (argc < 3) {
            terminal_print_message("[SYSTEM][ERROR] Specificare ID del task.\n", COLOR_RED, context);
            return;
        }
        int task_id = atoi(argv[2]);
        if (scheduler_resume_task(task_id) == SCHED_ERR_OK) {
            terminal_print_message("\n[SYSTEM] Task resumed.\n", COLOR_GREEN, context);
        } else {
            terminal_print_message("[SYSTEM][ERROR] ID task non valido.\n", COLOR_RED, context);
        }
    } else {
        terminal_print_message("[SYSTEM][ERROR] Sottocomando non riconosciuto.\n", COLOR_RED, context);
    }
}

void cmd_vt100(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        terminal_print_message("[SYSTEM][ERROR] Specificare lo stato (EN or DI).\n", COLOR_RED, context);
        return;
    }

    if (strcmp(argv[1], "EN") == 0) {
        context->enable_vt100_features = 1;
        terminal_print_message("[SYSTEM] VT100 abilitato.\n", COLOR_GREEN, context);
    } else if (strcmp(argv[1], "DI") == 0) {
        context->enable_vt100_features = 0;
        terminal_print_message("[SYSTEM] VT100 disabilitato.\n", COLOR_BLUE, context);
    } else {
        terminal_print_message("[SYSTEM][ERROR] Stato non valido. Usa EN per abilitare o DI per disabilitare.\n", COLOR_RED, context);
    }
}


void cmd_ps(terminal_context_t *context, size_t argc, char **argv) {
    scheduler_print_task_list();
}

void cmd_set_scheduler(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        terminal_print_message("[SYSTEM][ERROR] Specificare l'algoritmo. Opzioni: PRIORITY, ROUND_ROBIN, EARLIEST_DEADLINE_FIRST, LEAST_EXECUTED, LONGEST_WAITING\n", COLOR_RED, context);
        return;
    }

    sched_algorithm_t algorithm;

    if (strcmp(argv[1], "PRIORITY") == 0) {
        algorithm = SCHED_ALGO_PRIORITY;
    } else if (strcmp(argv[1], "ROUND_ROBIN") == 0) {
        algorithm = SCHED_ALGO_ROUND_ROBIN;
    } else if (strcmp(argv[1], "EARLIEST_DEADLINE_FIRST") == 0) {
        algorithm = SCHED_ALGO_EARLIEST_DEADLINE_FIRST;
    } else if (strcmp(argv[1], "LEAST_EXECUTED") == 0) {
        algorithm = SCHED_ALGO_LEAST_EXECUTED;
    } else if (strcmp(argv[1], "LONGEST_WAITING") == 0) {
        algorithm = SCHED_ALGO_LONGEST_WAITING;
    } else {
        terminal_print_message("[SYSTEM][ERROR] Algoritmo non valido. Usa HELP per vedere le opzioni.\n", COLOR_RED, context);
        return;
    }

    scheduler_set_algorithm(algorithm);
    terminal_print_message("[SYSTEM] Algoritmo dello scheduler aggiornato.\n", COLOR_GREEN, context);
}

void cmd_debug_task(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 3) {
        terminal_print_message("[SYSTEM][ERROR] Specificare l'ID del task e lo stato (EN or DI).\n", COLOR_RED, context);
        return;
    }

    int task_id = atoi(argv[1]);
    if (strcmp(argv[2], "EN") == 0) {
        debug_enable_task(task_id);
    } else if (strcmp(argv[2], "DI") == 0) {
        debug_disable_task(task_id);
    } else {
        terminal_print_message("Errore: stato non valido. Usa ENABLE o DISABLE.\n", COLOR_RED, context);
    }
}

void cmd_set(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 4) {
        terminal_print_message("Usage: SET <key> <type> <value>\n", COLOR_RED, context);
        return;
    }

    int key = atoi(argv[1]);
    param_type_t type;
    int result;

    if (strcmp(argv[2], "INT") == 0) {
        type = PARAM_TYPE_INT;
        char *endptr;
        int int_value = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0') {
            terminal_print_message("Invalid integer value.\n", COLOR_RED, context);
            return;
        }
        result = set_param(key, type, &int_value);
    } else if (strcmp(argv[2], "FLOAT") == 0) {
        type = PARAM_TYPE_FLOAT;
        char *endptr;
        float float_value = strtof(argv[3], &endptr);
        if (*endptr != '\0') {
            terminal_print_message("Invalid float value.\n", COLOR_RED, context);
            return;
        }
        result = set_param(key, type, &float_value);
    } else if (strcmp(argv[2], "STRING") == 0) {
        type = PARAM_TYPE_STRING;

        // Combina tutti gli argomenti successivi in una singola stringa
        char string_value[64] = {0};
        size_t len = 0;

        for (int i = 3; i < argc; i++) {
            if (len + strlen(argv[i]) + 1 >= sizeof(string_value)) {
                terminal_print_message("String too long. Truncated.\n", COLOR_YELLOW, context);
                break;
            }
            strncat(string_value, argv[i], sizeof(string_value) - len - 1);
            len = strlen(string_value);
            if (i < argc - 1) { // Aggiungi uno spazio tra gli argomenti
                strncat(string_value, " ", sizeof(string_value) - len - 1);
                len = strlen(string_value);
            }
        }

        result = set_param(key, type, string_value);
    } else {
        terminal_print_message("Invalid type. Use INT, FLOAT, or STRING.\n", COLOR_RED, context);
        return;
    }

    if (result == 0) {
        terminal_print_message("Parameter set successfully.\n", COLOR_GREEN, context);
    } else if (result == -2) {
        terminal_print_message("Value out of range.\n", COLOR_RED, context);
    } else {
        terminal_print_message("Failed to set parameter.\n", COLOR_RED, context);
    }
}

// Funzione cmd_list per mostrare solo i parametri inizializzati con statistiche
void cmd_list(terminal_context_t *context, size_t argc, char **argv) {
    terminal_print_message("Listing initialized parameters:\n", COLOR_BLUE, context);
    size_t total_params = 0;
    size_t int_count = 0, float_count = 0, string_count = 0;

    for (size_t i = 0; i < MAX_PARAMS; i++) {
       if (params[i].key >= 1 && params[i].type >= 0) { // Mostra solo parametri con chiave >= 1
            total_params++;
            char buffer[128];
            switch (params[i].type) {
                case PARAM_TYPE_INT:
                    int_count++;
                    snprintf(buffer, sizeof(buffer), "Key: %d, Type: INT, Value: %d\n", params[i].key, params[i].value.int_value);
                    break;
                case PARAM_TYPE_FLOAT:
                    float_count++;
                    snprintf(buffer, sizeof(buffer), "Key: %d, Type: FLOAT, Value: %.2f\n", params[i].key, params[i].value.float_value);
                    break;
                case PARAM_TYPE_STRING:
                    string_count++;
                    snprintf(buffer, sizeof(buffer), "Key: %d, Type: STRING, Value: %s\n", params[i].key, params[i].value.string_value);
                    break;
                default:
                    continue; // Salta tipi sconosciuti
            }
            terminal_print_message(buffer, COLOR_GREEN, context);
        }
    }

    size_t used_memory = get_config_params_memory_usage();
    float percentage_sram = ((float)used_memory / (256 * 1024)) * 100.0; // SRAM totale = 256 KB
    char mem_usage[128];
    snprintf(mem_usage, sizeof(mem_usage),
            "Total SRAM usage: %zu bytes (%.2f%% of available SRAM)\n",
            used_memory, percentage_sram);
    terminal_print_message(mem_usage, COLOR_YELLOW, context);

    size_t flash_capacity = FLASH_SECTOR_SIZE; // Supponendo una dimensione del settore
    size_t flash_used = sizeof(config_param_t) * MAX_PARAMS;
    float percentage_flash = ((float)flash_used / flash_capacity) * 100.0;
    char flash_usage[128];
    snprintf(flash_usage, sizeof(flash_usage),
            "Total Flash usage: %zu bytes (%.2f%% of reserved Flash)\n",
            flash_used, percentage_flash);
    terminal_print_message(flash_usage, COLOR_YELLOW, context);
}

void cmd_get(terminal_context_t *context, size_t argc, char **argv) {
    if (argc < 2) {
        terminal_print_message("Usage: GET <key>\n", COLOR_RED, context);
        return;
    }

    int key = atoi(argv[1]);
    config_param_t param;
    if (get_param(key, &param) == 0) {
        char buffer[128];
        switch (param.type) {
            case PARAM_TYPE_INT:
                snprintf(buffer, sizeof(buffer), "Key: %d, Type: INT, Value: %d\n", param.key, param.value.int_value);
                break;
            case PARAM_TYPE_FLOAT:
                snprintf(buffer, sizeof(buffer), "Key: %d, Type: FLOAT, Value: %.2f\n", param.key, param.value.float_value);
                break;
            case PARAM_TYPE_STRING:
                snprintf(buffer, sizeof(buffer), "Key: %d, Type: STRING, Value: %s\n", param.key, param.value.string_value);
                break;
        }
        terminal_print_message(buffer, COLOR_GREEN, context);
    } else {
        char error[64];
        snprintf(error, sizeof(error), "Parameter with key %d not found.\n", key);
        terminal_print_message(error, COLOR_RED, context);
    }
}

void cmd_reset(terminal_context_t *context, size_t argc, char **argv) {
    reset_params_to_defaults();
    terminal_print_message("Parameters reset to defaults.\n", COLOR_GREEN, context);
}

void init_commands(terminal_context_t *context) {
    terminal_register_command(context, "HELP", "Mostra la lista dei comandi", cmd_help);
    terminal_register_command(context, "HISTORY", "Mostra lo storico dei comandi", cmd_history);
    terminal_register_command(context, "LOGIN", "Effettua il login", cmd_login);
    terminal_register_command(context, "LOGOUT", "Disconnette l'utente", cmd_logout);
    terminal_register_command(context, "TASK", "Gestisce i task (PRIO, HOLD, RUN)", cmd_tasks);
    terminal_register_command(context, "VT100", "Abilita/disabilita VT100 (es. VT100 EN or DI)", cmd_vt100);
    terminal_register_command(context, "PS", "Mostra la lista dei task attivi", cmd_ps);
    terminal_register_command(context, "REBOOT", "Riavvia il dispositivo", cmd_reboot);
    terminal_register_command(context, "ALG", "Cambia l'algoritmo dello scheduler (es. ALG RR)", cmd_set_scheduler);
    terminal_register_command(context, "DBG", "Abilita/disabilita il debug per un task (es. DBG <id> EN or DI)", cmd_debug_task);
    terminal_register_command(context, "SET", "Set a parameter", cmd_set);
    terminal_register_command(context, "GET", "Get a parameter", cmd_get);
    terminal_register_command(context, "LIST", "List all parameters", cmd_list);
    terminal_register_command(context, "RESET", "Reset parameters to defaults", cmd_reset);
}

