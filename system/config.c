#include "config.h"
#include "flash.h"
#include <string.h>
#include <stdio.h>

// Parametri globali
config_param_t params[MAX_PARAMS];

// Dichiarazione di default_params (definito in secret.c)
extern const config_param_t default_params[MAX_PARAMS];


void init_params() {
    memcpy(params, default_params, sizeof(params));
}

size_t get_config_params_memory_usage() {
    return sizeof(config_param_t) * MAX_PARAMS;
}

// Funzione di validazione migliorata
bool validate_param(const config_param_t *param) {
    switch (param->type) {
        case PARAM_TYPE_INT:
            return param->value.int_value >= param->validation.int_range.min &&
                   param->value.int_value <= param->validation.int_range.max;
        case PARAM_TYPE_FLOAT:
            return param->value.float_value >= param->validation.float_range.min &&
                   param->value.float_value <= param->validation.float_range.max;
        case PARAM_TYPE_STRING:
            return strlen(param->value.string_value) > 0 &&
                   strlen(param->value.string_value) < sizeof(param->value.string_value);
        default:
            return false;
    }
}

// Funzione set_param per gestire conversioni e formati
int set_param(int key, param_type_t type, void *value) {
    for (size_t i = 0; i < MAX_PARAMS; i++) {
        if (params[i].key == key) {
            if (params[i].type != type) {
                return -1; // Tipo errato
            }

            // Validazione preliminare del valore
            config_param_t temp_param = params[i];
            switch (type) {
                case PARAM_TYPE_INT: {
                    int int_value = *(int *)value;
                    temp_param.value.int_value = int_value;
                    if (!validate_param(&temp_param)) {
                        return -2; // Valore non valido
                    }
                    params[i].value.int_value = int_value;
                    break;
                }
                case PARAM_TYPE_FLOAT: {
                    float float_value = *(float *)value;
                    temp_param.value.float_value = float_value;
                    if (!validate_param(&temp_param)) {
                        return -2; // Valore non valido
                    }
                    params[i].value.float_value = float_value;
                    break;
                }
                case PARAM_TYPE_STRING: {
                    char *str_value = (char *)value;
                    strncpy(temp_param.value.string_value, str_value, sizeof(temp_param.value.string_value) - 1);
                    temp_param.value.string_value[sizeof(temp_param.value.string_value) - 1] = '\0';
                    if (!validate_param(&temp_param)) {
                        return -2; // Valore non valido
                    }
                    strncpy(params[i].value.string_value, str_value, sizeof(params[i].value.string_value) - 1);
                    params[i].value.string_value[sizeof(params[i].value.string_value) - 1] = '\0';
                    break;
                }
                default:
                    return -1;
            }
            return 0;
        }
    }
    return -1; // Parametro non trovato
}

int get_param(int key, config_param_t *out_param) {
    for (size_t i = 0; i < MAX_PARAMS; i++) {
        if (params[i].key == key) {
            *out_param = params[i];
            return 0;
        }
    }
    return -1; // Parametro non trovato
}

void reset_params_to_defaults() {
    memcpy(params, default_params, sizeof(params));
}

void save_params_to_flash() {
    flash_storage_write(params, sizeof(params));
}

void load_params_from_flash() {
    flash_storage_read(params, sizeof(params));
}
