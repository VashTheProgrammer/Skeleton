#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdio.h>

// Macro for task-specific debug logging
#define DEBUG_LOG_TASK(task_id, format, ...) \
    do { \
        if (debug_is_task_enabled(task_id)) { \
            printf("[DEBUG][Task %d] " format "\n", task_id, ##__VA_ARGS__); \
        } \
    } while (0)

// API for debug control
void debug_enable_task(int task_id);
void debug_disable_task(int task_id);
bool debug_is_task_enabled(int task_id);

#endif // DEBUG_H
