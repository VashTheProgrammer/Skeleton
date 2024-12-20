#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "scheduler.h"

// -----------------------------------------------------------------------------
// Variables for State and Statistics
// -----------------------------------------------------------------------------
static task_t task_list[MAX_TASKS]; // List of all tasks
static int task_count = 0; // Total number of tasks
static sched_algorithm_t selected_algorithm = SCHED_ALGO_ROUND_ROBIN; // Current scheduling algorithm
static int priority_normalization_counter = 0; // Counter for priority normalization
static int64_t global_total_task_time = 0; // Total execution time of all tasks

// Stack for each task
static uint8_t task_stacks[MAX_TASKS][TASK_STACK_SIZE];

// Forward declaration of scheduling algorithms
// Explains their role in task selection and operation
static int find_highest_priority_task(absolute_time_t current_time);
static int find_round_robin_task(absolute_time_t current_time);
static int find_earliest_deadline_task(absolute_time_t current_time);
static int find_least_executed_task(absolute_time_t current_time);
static int find_longest_waiting_task(absolute_time_t current_time);

// Array of scheduling algorithm functions indexed by the algorithm type
// Used dynamically to invoke the correct algorithm based on configuration
typedef int (*sched_func_t)(absolute_time_t current_time);
static sched_func_t sched_algorithms[] = {
    find_highest_priority_task,     // PRIORITY algorithm
    find_round_robin_task,          // ROUND_ROBIN algorithm
    find_earliest_deadline_task,    // EARLIEST_DEADLINE_FIRST algorithm
    find_least_executed_task,       // LEAST_EXECUTED algorithm
    find_longest_waiting_task       // LONGEST_WAITING algorithm
};

// Stack usage monitoring functions:
// Used to analyze memory usage and ensure efficient stack utilization.

// Fills a task stack with a known pattern for tracking memory usage.
static void initialize_task_stack(uint8_t *stack, size_t size) {
    // Inizializza lo stack con un valore noto per monitorare l'uso dello stack.
    memset(stack, STACK_FILL_VALUE, size);
}

// Calculates the amount of stack memory used by a task.
// Searches for the first byte not matching the fill value.
static int calculate_stack_usage(const uint8_t *stack, size_t size) {
    // Calcola l'uso dello stack trovando la prima posizione diversa dal valore noto.
    int used = 0;
    while (used < (int)size && stack[used] != STACK_FILL_VALUE) {
        used++;
    }
    return used;
}

// -----------------------------------------------------------------------------
// Task Management Functions
// -----------------------------------------------------------------------------

// Adds a task to the scheduler
// This function initializes and registers a new task in the scheduler's task list.
sched_error_t scheduler_add_task(const char *name, task_func_t task, int priority, int64_t interval, task_state_t state, size_t static_memory_size) {
    if (task_count >= MAX_TASKS) {
        return SCHED_ERR_FULL; // Maximum number of tasks reached
    }
    if (task == NULL || interval <= 0) {
        return SCHED_ERR_INVALID_PARAMS; // Invalid task parameters
    }

    task_t *t = &task_list[task_count];
    memset(t, 0, sizeof(task_t)); // Clear the task structure
    t->task = task;
    t->state = state;
    t->priority = priority;
    t->dynamic_priority = priority; // Initialize dynamic priority
    t->interval = interval;
    t->last_execution = make_timeout_time_us(0); // Initialize last execution time
    t->name = name;
    t->min_exec_time = INT64_MAX; // Initialize to track the minimum execution time
    t->memory_allocated = static_memory_size; // Record allocated memory

    initialize_task_stack(task_stacks[task_count], TASK_STACK_SIZE); // Prepare the task stack

    task_count++;
    return SCHED_ERR_OK;
}

// Updates the priority of an existing task
sched_error_t scheduler_set_task_priority(int task_index, int new_priority) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].priority = new_priority;
    task_list[task_index].dynamic_priority = new_priority; // Update dynamic priority
    return SCHED_ERR_OK;
}

// Sets the execution interval of a task
sched_error_t scheduler_set_task_interval(int task_index, int64_t new_interval) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].interval = new_interval;
    return SCHED_ERR_OK;
}

// Pauses a task
// When paused, the task's statistics are reset to avoid incorrect jitter calculations.
sched_error_t scheduler_pause_task(int task_index) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].state = TASK_PAUSED;

    task_list[task_index].total_jitter = 0; // Reset jitter statistics
    task_list[task_index].max_jitter = 0;
    task_list[task_index].last_execution = get_absolute_time(); // Update last execution time

    return SCHED_ERR_OK;
}

// Resumes a paused task
// Resets jitter statistics to ensure accurate calculations upon resumption.
sched_error_t scheduler_resume_task(int task_index) {
    if (task_index < 0 || task_index >= task_count) return SCHED_ERR_INVALID_INDEX;
    task_list[task_index].state = TASK_RUNNING;

    task_list[task_index].total_jitter = 0; // Reset jitter statistics
    task_list[task_index].max_jitter = 0;
    task_list[task_index].last_execution = get_absolute_time(); // Update last execution time

    return SCHED_ERR_OK;
}

// Changes the active scheduling algorithm
// This function pauses all tasks, updates the scheduler algorithm, resets statistics,
// and resumes tasks in a consistent state.
sched_error_t scheduler_set_algorithm(sched_algorithm_t algorithm) {
    // Pause all tasks to avoid conflicts during algorithm change
    for (int i = 0; i < task_count; i++) {
        task_list[i].state = TASK_PAUSED;
    }

    // Change the scheduler's algorithm
    selected_algorithm = algorithm;

    // Reset all task statistics to ensure accurate data under the new algorithm
    for (int i = 0; i < task_count; i++) {
        task_t *t = &task_list[i];
        t->exec_count = 0;            // Reset execution count
        t->total_time = 0;            // Reset cumulative execution time
        t->total_exec_time = 0;       // Reset total execution time
        t->max_exec_time = 0;         // Reset maximum execution time
        t->min_exec_time = INT64_MAX; // Reset minimum execution time
        t->total_jitter = 0;          // Reset total jitter
        t->max_jitter = 0;            // Reset maximum jitter
        t->last_execution = get_absolute_time(); // Update the last execution timestamp
    }

    // Resume all tasks after reconfiguration
    for (int i = 0; i < task_count; i++) {
        task_list[i].state = TASK_RUNNING;
    }

    return SCHED_ERR_OK;
}


// -----------------------------------------------------------------------------
// Dynamic Priority Normalization
// -----------------------------------------------------------------------------

// Normalizes dynamic priorities to their static values
// This process ensures that the PRIORITY scheduling algorithm functions correctly.
// Without normalization, dynamic priorities could diverge significantly, causing tasks
// with inherently lower priorities to suffer from starvation. By resetting all dynamic
// priorities to their static values periodically, this mechanism prevents such issues
// and maintains fairness among tasks.
static void normalize_dynamic_priorities(void) {
    for (int i = 0; i < task_count; i++) {
        task_list[i].dynamic_priority = task_list[i].priority; // Reset to static priority
    }
}

// -----------------------------------------------------------------------------
// Scheduling Algorithm Implementations
// -----------------------------------------------------------------------------

// PRIORITY Algorithm
// Selects the task with the highest dynamic priority that is ready to run.
// This algorithm is well-suited for systems where tasks have clearly defined
// priority levels. However, without periodic normalization, lower-priority
// tasks could experience starvation.
static int find_highest_priority_task(absolute_time_t current_time) {
    int highest_priority_index = -1;
    int highest_priority = -1;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        int64_t since_last = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (task_list[i].task && since_last >= task_list[i].interval) {
            if (task_list[i].dynamic_priority > highest_priority) {
                highest_priority = task_list[i].dynamic_priority;
                highest_priority_index = i;
            }
        }
    }

    // Normalize priorities periodically to prevent starvation.
    if (++priority_normalization_counter >= PRIORITY_NORMALIZATION_INTERVAL) {
        normalize_dynamic_priorities();
        priority_normalization_counter = 0;
    }

    return highest_priority_index;
}

// ROUND-ROBIN Algorithm
// Cycles through tasks in a fixed order, ensuring all tasks get a turn to run.
// This algorithm is simple and fair but does not account for task priority
// or varying workloads, making it less suitable for real-time systems.
static int find_round_robin_task(absolute_time_t current_time) {
    static int last_task_index = -1;
    for (int i = 0; i < task_count; i++) {
        int current_index = (last_task_index + 1 + i) % task_count;
        if (task_list[current_index].state == TASK_PAUSED) continue;
        int64_t since_last = absolute_time_diff_us(task_list[current_index].last_execution, current_time);
        if (task_list[current_index].task && since_last >= task_list[current_index].interval) {
            last_task_index = current_index;
            return current_index;
        }
    }
    return -1;
}

// EARLIEST-DEADLINE-FIRST Algorithm
// Prioritizes tasks based on their deadlines, executing the task with the
// earliest deadline first. This algorithm is ideal for systems with hard
// deadlines, but requires accurate deadline tracking and scheduling.
static int find_earliest_deadline_task(absolute_time_t current_time) {
    int earliest_index = -1;
    int64_t earliest_deadline = INT64_MAX;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        int64_t deadline = to_us_since_boot(task_list[i].last_execution) + task_list[i].interval;
        int64_t since_last = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (task_list[i].task && since_last >= task_list[i].interval && deadline < earliest_deadline) {
            earliest_deadline = deadline;
            earliest_index = i;
        }
    }

    return earliest_index;
}

// LEAST-EXECUTED Algorithm
// Executes the task with the lowest execution count, balancing workload
// distribution across tasks. This approach is effective for systems where
// all tasks are of equal importance and should share CPU time equally.
static int find_least_executed_task(absolute_time_t current_time) {
    (void)current_time; // Not required for this algorithm
    int least_executed_index = -1;
    int min_exec_count = INT_MAX;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        if (task_list[i].exec_count < min_exec_count) {
            min_exec_count = task_list[i].exec_count;
            least_executed_index = i;
        }
    }

    return least_executed_index;
}

// LONGEST-WAITING Algorithm
// Executes the task that has been waiting the longest since its last execution.
// This algorithm is effective for reducing task latency but may not suit systems
// where task priority or deadlines are critical.
static int find_longest_waiting_task(absolute_time_t current_time) {
    int longest_waiting_index = -1;
    int64_t max_wait_time = -1;

    for (int i = 0; i < task_count; i++) {
        if (task_list[i].state == TASK_PAUSED) continue;
        int64_t wait_time = absolute_time_diff_us(task_list[i].last_execution, current_time);
        if (wait_time > max_wait_time) {
            max_wait_time = wait_time;
            longest_waiting_index = i;
        }
    }

    return longest_waiting_index;
}

static int select_next_task(absolute_time_t current_time) {
    int algo_index = (int)selected_algorithm;
    if (algo_index < 0 || algo_index >= (int)(sizeof(sched_algorithms)/sizeof(sched_algorithms[0]))) {
        return -1;
    }
    return sched_algorithms[algo_index](current_time);
}

// -----------------------------------------------------------------------------
// Scheduler Core Function: scheduler_run
// -----------------------------------------------------------------------------
// The core loop of the scheduler. This function continuously selects and executes tasks
// based on the active scheduling algorithm. It also updates task statistics and
// calculates execution metrics such as jitter and execution time.
//
// Key Operations:
// 1. Get the current system time.
// 2. Select the next task to execute using the active algorithm.
// 3. If a task is selected, calculate jitter, update execution metrics, and execute the task.
// 4. If no task is executable, the scheduler idles momentarily.

void scheduler_run(void) {
    while (1) {
        absolute_time_t current_time = get_absolute_time();
        int task_index = select_next_task(current_time);

        if (task_index != -1) {
            task_t *t = &task_list[task_index];

            // Calculate time since last execution
            int64_t since_last = absolute_time_diff_us(t->last_execution, current_time);
            
            // Calculate jitter (deviation from the ideal interval)
            int64_t jitter = since_last - t->interval;
            if (jitter < 0) jitter = -jitter;
            t->total_jitter += jitter;
            if (jitter > t->max_jitter) {
                t->max_jitter = jitter;
            }

            // Execute the task and measure execution time
            absolute_time_t start_time = current_time;
            t->task(); // Task execution
            absolute_time_t end_time = get_absolute_time();

            t->last_execution = end_time; // Update last execution time
            t->dynamic_priority = t->priority; // Reset dynamic priority
            t->exec_count++; // Increment execution count

            // Calculate execution time for the task
            int64_t exec_time = absolute_time_diff_us(start_time, end_time);
            t->total_time += exec_time;
            t->total_exec_time += exec_time;
            if (exec_time > t->max_exec_time) t->max_exec_time = exec_time;
            if (exec_time < t->min_exec_time) t->min_exec_time = exec_time;

            global_total_task_time += exec_time; // Update global task time
        } else {
            // No executable task, introduce a small idle delay
            // sleep_us(100);
        }
    }
}

// -----------------------------------------------------------------------------
// Detailed Explanation: scheduler_print_task_list
// -----------------------------------------------------------------------------
// This function provides a comprehensive report of all tasks managed by the scheduler.
// It prints detailed metrics that help understand task performance and resource usage.
//
// Metrics Explained:
// 1. **PID (Process ID):** Unique identifier for each task.
// 2. **Name:** Descriptive name of the task.
// 3. **State:** Indicates whether the task is RUNNING or PAUSED.
// 4. **Priority:** The task's static priority, used by the PRIORITY scheduling algorithm.
// 5. **ExecCount:** Number of times the task has been executed.
// 6. **TotalTime:** Total execution time of the task in microseconds.
// 7. **MinTime:** Shortest recorded execution time of the task.
// 8. **MaxTime:** Longest recorded execution time of the task.
// 9. **AvgTime:** Average execution time per run.
// 10. **MaxJitter:** The maximum observed jitter (difference from ideal interval).
// 11. **AvgJitter:** The average jitter over all executions.
// 12. **MemUsed:** Combined stack usage and statically allocated memory for the task.
//
// These metrics are useful for identifying performance bottlenecks, ensuring tasks
// meet timing constraints, and analyzing resource utilization.

const char* scheduler_algorithm_to_string(sched_algorithm_t algorithm) {
    switch (algorithm) {
        case SCHED_ALGO_PRIORITY: return "PRIORITY";
        case SCHED_ALGO_ROUND_ROBIN: return "ROUND_ROBIN";
        case SCHED_ALGO_EARLIEST_DEADLINE_FIRST: return "EARLIEST_DEADLINE_FIRST";
        case SCHED_ALGO_LEAST_EXECUTED: return "LEAST_EXECUTED";
        case SCHED_ALGO_LONGEST_WAITING: return "LONGEST_WAITING";
        default: return "UNKNOWN";
    }
}

static void print_task_info(int index, const task_t *task, int stack_used) {
    printf("%-5d %-10s %-10s %-10d %-10d %-10lld %-10lld %-10lld %-10lld %-10lld %-10lld %-10zu\n",
           index, // PID
           task->name,
           task->state == TASK_RUNNING ? "RUNNING" : "PAUSED",
           task->priority, // Static Priority
           task->exec_count, // Execution Count
           task->total_time, // Total Execution Time
           (task->min_exec_time == INT64_MAX) ? 0 : task->min_exec_time, // Min Execution Time
           task->max_exec_time, // Max Execution Time
           task->exec_count > 0 ? (task->total_exec_time / task->exec_count) : 0, // Average Execution Time
           task->max_jitter,
           task->exec_count > 0 ? (task->total_jitter / task->exec_count) : 0,
           stack_used + task->memory_allocated); // Memory Used
}


void scheduler_print_task_list(void) {
    const char *algo_name = scheduler_algorithm_to_string(selected_algorithm);

    int64_t current_system_time = to_us_since_boot(get_absolute_time());
    double cpu_usage_percentage = ((double)global_total_task_time / (double)current_system_time) * 100.0;

    // Calculate total memory usage
    size_t total_memory_usage = 0;
    for (int i = 0; i < task_count; i++) {
        int stack_used = calculate_stack_usage(task_stacks[i], TASK_STACK_SIZE);
        total_memory_usage += (stack_used + task_list[i].memory_allocated);
    }
    double memory_usage_percentage = ((double)total_memory_usage / (double)RP2040_TOTAL_RAM) * 100.0;

    // Print global statistics
    printf("\n--- Global Task Statistics ---\n");
    printf("Scheduler Algorithm: %s\n", algo_name);
    printf("CPU Usage: %.2f%% (%lld us)\n", cpu_usage_percentage, global_total_task_time);
    printf("Total System Time: %lld us\n", current_system_time);
    printf("Total Memory Usage: %zu bytes (%.2f%% of total 264KB RAM)\n\n", total_memory_usage, memory_usage_percentage);

    printf("%-5s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
       "PID", "Name", "State", "Priority", "ExecCount", "TotalTime",
       "MinTime", "MaxTime", "AvgTime", "MaxJitter", "AvgJitter", "MemUsed");

    for (int i = 0; i < task_count; i++) {
        int stack_used = calculate_stack_usage(task_stacks[i], TASK_STACK_SIZE);
        print_task_info(i, &task_list[i], stack_used);
    }
    printf("\n");
}
