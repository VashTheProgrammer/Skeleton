#ifndef INITCALLS_H
#define INITCALLS_H

#define MAX_INITCALLS 32

// Structure for initialization functions
typedef struct initcall_node {
    void (*init_func)(void); // Pointer to the initialization function
} initcall_node_t;

// Array of initialization calls and counter
extern initcall_node_t initcall_list[MAX_INITCALLS];
extern int initcall_count;

// Macro to register an initialization function
#define REGISTER_INITCALL(func) \
    static void __attribute__((constructor)) register_##func(void) { \
        if (initcall_count < MAX_INITCALLS) { \
            initcall_list[initcall_count++].init_func = func; \
        } \
    }

// Executes all registered initialization functions
void initcalls(void);

#endif // INITCALLS_H

