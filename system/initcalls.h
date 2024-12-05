#ifndef INITCALLS_H
#define INITCALLS_H

#define MAX_INITCALLS 32

typedef struct initcall_node {
    void (*init_func)(void);  // Puntatore alla funzione di inizializzazione
} initcall_node_t;

// Array di initcalls e contatore
extern initcall_node_t initcall_list[MAX_INITCALLS];
extern int initcall_count;

// Macro per registrare una funzione di inizializzazione
#define REGISTER_INITCALL(func) \
    static void __attribute__((constructor)) register_##func(void) { \
        if (initcall_count < MAX_INITCALLS) { \
            initcall_list[initcall_count++].init_func = func; \
        } \
    }

void initcalls(void);

#endif // INITCALLS_H
