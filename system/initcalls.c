#include "initcalls.h"

// Inizializza la testa della lista a NULL
initcall_node_t initcall_list[MAX_INITCALLS];
int initcall_count = 0;

void initcalls(void)
{
    // Itera sull'array delle initcall ed esegui ogni funzione
    for (int i = 0; i < initcall_count; i++) {
        initcall_list[i].init_func();
    }
}