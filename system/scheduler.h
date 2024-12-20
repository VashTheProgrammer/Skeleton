#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/time.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define RP2040_TOTAL_RAM        (264 * 1024) // Total RAM of RP2040 (264 KB = 270336 bytes)
#define STACK_FILL_VALUE        (0xAA)       // Value used to fill task stacks for usage monitoring
#define TASK_STACK_SIZE         1024         // Stack size for each task in bytes
#define MAX_TASKS               10           // Maximum number of tasks supported
#define PRIORITY_NORMALIZATION_INTERVAL 100  // Interval for priority normalization (in iterations)

// -----------------------------------------------------------------------------
// Definitions and Types
// -----------------------------------------------------------------------------

// Task states
typedef enum {
    TASK_RUNNING, // Task is ready and running
    TASK_PAUSED   // Task is paused and not executing
} task_state_t;

// Scheduler error codes
typedef enum {
    SCHED_ERR_OK = 0,          // No error
    SCHED_ERR_FULL,            // Maximum number of tasks reached
    SCHED_ERR_INVALID_INDEX,   // Invalid task index provided
    SCHED_ERR_INVALID_PARAMS   // Invalid parameters for task or function
} sched_error_t;

// Scheduling algorithms
typedef enum {
    SCHED_ALGO_PRIORITY,               // Priority-based scheduling
    SCHED_ALGO_ROUND_ROBIN,            // Round-robin scheduling
    SCHED_ALGO_EARLIEST_DEADLINE_FIRST, // Earliest deadline first
    SCHED_ALGO_LEAST_EXECUTED,         // Least executed task scheduling
    SCHED_ALGO_LONGEST_WAITING         // Longest waiting task scheduling
} sched_algorithm_t;

// Task function type
typedef void (*task_func_t)(void); // Function pointer type for task functions

// Task structure
typedef struct {
    const char *name;                // Name of the task
    task_func_t task;                // Function to execute as the task
    int priority;                    // Static priority of the task
    int dynamic_priority;            // Dynamic priority used in scheduling
    int state;                       // Current state (running or paused)
    int64_t interval;                // Execution interval in microseconds
    absolute_time_t last_execution;  // Timestamp of the last execution
    int exec_count;                  // Number of times the task has executed
    int64_t total_time;              // Cumulative execution time of the task
    int64_t total_exec_time;         // Total execution time across all runs
    int64_t max_exec_time;           // Maximum recorded execution time
    int64_t max_jitter;              // Maximum recorded jitter
    int64_t min_exec_time;           // Minimum recorded execution time
    int64_t total_jitter;            // Cumulative jitter across executions
    size_t memory_allocated;         // Static memory allocated to the task
} task_t;

// -----------------------------------------------------------------------------
// Scheduler API
// -----------------------------------------------------------------------------

// Adds a new task to the scheduler
sched_error_t scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval, task_state_t state, size_t static_memory_size);

// Sets the priority of a specific task
sched_error_t scheduler_set_task_priority(int task_index, int new_priority);

// Sets the interval of a specific task
sched_error_t scheduler_set_task_interval(int task_index, int64_t new_interval);

// Pauses a specific task
sched_error_t scheduler_pause_task(int task_index);

// Resumes a specific task
sched_error_t scheduler_resume_task(int task_index);

// Changes the active scheduling algorithm
sched_error_t scheduler_set_algorithm(sched_algorithm_t algorithm);

// Retrieves the currently active scheduling algorithm
sched_algorithm_t scheduler_get_algorithm(void);

// Main loop of the scheduler that manages task execution
void scheduler_run(void);

// Prints detailed statistics and information about all tasks
void scheduler_print_task_list(void);

#endif // SCHEDULER_H
