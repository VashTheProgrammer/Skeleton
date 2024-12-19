#include <stdio.h>
#include <stdbool.h>

#include "debug.h"
#include <scheduler.h>

// Stato del debug per ciascun task
static bool debug_task_enabled[MAX_TASKS] = {false};

// Abilita il debug per un task specifico
void debug_enable_task(int task_id) {
    if (task_id >= 0 && task_id < MAX_TASKS) {
        debug_task_enabled[task_id] = true;
        printf("[DEBUG] Debug abilitato per il task %d\n", task_id);
    }
}

// Disabilita il debug per un task specifico
void debug_disable_task(int task_id) {
    if (task_id >= 0 && task_id < MAX_TASKS) {
        debug_task_enabled[task_id] = false;
        printf("[DEBUG] Debug disabilitato per il task %d\n", task_id);
    }
}

// Verifica se il debug è abilitato per un task specifico
bool debug_is_task_enabled(int task_id) {
    if (task_id >= 0 && task_id < MAX_TASKS) {
        return debug_task_enabled[task_id];
    }
    return false;
}
