#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_PARAMS 20

// Tipi di parametri
typedef enum {
    PARAM_TYPE_INT,
    PARAM_TYPE_FLOAT,
    PARAM_TYPE_STRING
} param_type_t;

typedef struct {
    int key;                   // ID univoco del parametro
    param_type_t type;         // Tipo del parametro
    union {
        int int_value;
        float float_value;
        char string_value[64];
    } value;                   // Valore del parametro
    union {
        struct { int min; int max; } int_range;
        struct { float min; float max; } float_range;
    } validation;              // Range di validazione
} config_param_t;

// Dichiarazione globale dei parametri
extern config_param_t params[MAX_PARAMS];

// Funzioni principali
void init_params();
size_t get_config_params_memory_usage();
bool validate_param(const config_param_t *param);
int set_param(int key, param_type_t type, void *value);
int get_param(int key, config_param_t *out_param);
void reset_params_to_defaults();
void save_params_to_flash();
void load_params_from_flash();

#endif // CONFIG_H