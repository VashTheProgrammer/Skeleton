#include "initcalls.h"

// Initializes the head of the list to NULL
initcall_node_t initcall_list[MAX_INITCALLS];
int initcall_count = 0;

// Executes all registered initialization functions
void initcalls(void) {
    for (int i = 0; i < initcall_count; i++) {
        initcall_list[i].init_func(); // Execute each initialization function
    }
}
