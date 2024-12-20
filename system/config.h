#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_PARAMS 20

// Types of configuration parameters
typedef enum {
    PARAM_TYPE_INT,
    PARAM_TYPE_FLOAT,
    PARAM_TYPE_STRING
} param_type_t;

// Structure defining a configuration parameter
typedef struct {
    int key;                   // Unique ID of the parameter
    param_type_t type;         // Type of the parameter
    union {
        int int_value;
        float float_value;
        char string_value[64];
    } value;                   // Value of the parameter
    union {
        struct { int min; int max; } int_range;
        struct { float min; float max; } float_range;
    } validation;              // Validation range
} config_param_t;

// Global parameter declarations
extern config_param_t params[MAX_PARAMS];

// Main configuration functions
void init_params();
size_t get_config_params_memory_usage();
bool validate_param(const config_param_t *param);
int set_param(int key, param_type_t type, void *value);
int get_param(int key, config_param_t *out_param);
void reset_params_to_defaults();
void save_params_to_flash();
void load_params_from_flash();

/*
 * Note: This configuration system relies on a `secret.c` file where
 * default parameter values are defined. Ensure that this file is linked
 * correctly during the build process.
 */

#endif // CONFIG_H
