#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/watchdog.h"
#include "terminal/cmd.h"
#include "scheduler.h"

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
        terminal_print_message("[SYSTEM][ERROR] Specificare un sottocomando (SETPRIO, HOLD, RESUME).\n", COLOR_RED, context);
        return;
    }

    if (strcmp(argv[1], "PS") == 0) {
        scheduler_print_task_list();
    } else if (strcmp(argv[1], "SETPRIO") == 0) {
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
    } else if (strcmp(argv[1], "RESUME") == 0) {
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

void cmd_enable_vt100(terminal_context_t *context, size_t argc, char **argv) {
    context->enable_vt100_features = 1;
    terminal_print_message("[SYSTEM] VT100 abilitato.\n", COLOR_GREEN, context);
}

void cmd_disable_vt100(terminal_context_t *context, size_t argc, char **argv) {
    context->enable_vt100_features = 0;
    terminal_print_message("[SYSTEM] VT100 disabilitato.\n", COLOR_BLUE, context);
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

void init_commands(terminal_context_t *context) {
    terminal_register_command(context, "HELP", "Mostra la lista dei comandi", cmd_help);
    terminal_register_command(context, "HISTORY", "Mostra lo storico dei comandi", cmd_history);
    terminal_register_command(context, "LOGIN", "Effettua il login", cmd_login);
    terminal_register_command(context, "LOGOUT", "Disconnette l'utente", cmd_logout);
    terminal_register_command(context, "TASK", "Gestisce i task (SETPRIO, HOLD, RESUME)", cmd_tasks);
    terminal_register_command(context, "ENABLE_VT100", "Abilita funzionalità VT100", cmd_enable_vt100);
    terminal_register_command(context, "DISABLE_VT100", "Disabilita funzionalità VT100", cmd_disable_vt100);
    terminal_register_command(context, "PS", "Mostra la lista dei task attivi", cmd_ps);
    terminal_register_command(context, "REBOOT", "Riavvia il dispositivo", cmd_reboot);
    terminal_register_command(context, "SET_ALGO", "Cambia l'algoritmo dello scheduler (es. SET_ALGO ROUND_ROBIN)", cmd_set_scheduler);
}

