#include <stdio.h>
#include <stdbool.h>

#include "debug.h"
#include "scheduler.h"

// Debug state for each task
static bool debug_task_enabled[MAX_TASKS] = {false};

// Enables debug for a specific task
void debug_enable_task(int task_id) {
    if (task_id >= 0 && task_id < MAX_TASKS) {
        debug_task_enabled[task_id] = true;
        printf("[DEBUG] Debug enabled for task %d\n", task_id);
    }
}

// Disables debug for a specific task
void debug_disable_task(int task_id) {
    if (task_id >= 0 && task_id < MAX_TASKS) {
        debug_task_enabled[task_id] = false;
        printf("[DEBUG] Debug disabled for task %d\n", task_id);
    }
}

// Checks if debug is enabled for a specific task
bool debug_is_task_enabled(int task_id) {
    if (task_id >= 0 && task_id < MAX_TASKS) {
        return debug_task_enabled[task_id];
    }
    return false;
}
