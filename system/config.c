#include "config.h"
#include "flash.h"
#include <string.h>
#include <stdio.h>

// Global configuration parameters
config_param_t params[MAX_PARAMS];

// Declaration of default_params (defined in secret.c)
extern const config_param_t default_params[MAX_PARAMS];

// Initializes configuration parameters with default values
void init_params() {
    memcpy(params, default_params, sizeof(params));
}

// Returns memory usage of configuration parameters
size_t get_config_params_memory_usage() {
    return sizeof(config_param_t) * MAX_PARAMS;
}

// Validates a parameter based on its type and validation rules
bool validate_param(const config_param_t *param) {
    // Check the parameter type and apply the appropriate validation logic
    switch (param->type) {
        case PARAM_TYPE_INT:
            // Ensure the integer value is within the defined range
            return param->value.int_value >= param->validation.int_range.min &&
                   param->value.int_value <= param->validation.int_range.max;
        case PARAM_TYPE_FLOAT:
            // Ensure the float value is within the defined range
            return param->value.float_value >= param->validation.float_range.min &&
                   param->value.float_value <= param->validation.float_range.max;
        case PARAM_TYPE_STRING:
            // Ensure the string is non-empty and fits within the allocated size
            return strlen(param->value.string_value) > 0 &&
                   strlen(param->value.string_value) < sizeof(param->value.string_value);
        default:
            // Return false for unsupported parameter types
            return false;
    }
}

// Sets a parameter value, validating it before applying
int set_param(int key, param_type_t type, void *value) {
    for (size_t i = 0; i < MAX_PARAMS; i++) {
        if (params[i].key == key) {
            if (params[i].type != type) {
                // Return error if the type does not match the expected type
                return -1; // Type mismatch
            }

            // Create a temporary copy of the parameter for validation
            config_param_t temp_param = params[i];
            switch (type) {
                case PARAM_TYPE_INT: {
                    int int_value = *(int *)value;
                    temp_param.value.int_value = int_value;
                    // Validate the new value before applying
                    if (!validate_param(&temp_param)) {
                        return -2; // Invalid value
                    }
                    // Apply the validated value
                    params[i].value.int_value = int_value;
                    break;
                }
                case PARAM_TYPE_FLOAT: {
                    float float_value = *(float *)value;
                    temp_param.value.float_value = float_value;
                    // Validate the new value before applying
                    if (!validate_param(&temp_param)) {
                        return -2; // Invalid value
                    }
                    // Apply the validated value
                    params[i].value.float_value = float_value;
                    break;
                }
                case PARAM_TYPE_STRING: {
                    char *str_value = (char *)value;
                    strncpy(temp_param.value.string_value, str_value, sizeof(temp_param.value.string_value) - 1);
                    temp_param.value.string_value[sizeof(temp_param.value.string_value) - 1] = '\0';
                    // Validate the new value before applying
                    if (!validate_param(&temp_param)) {
                        return -2; // Invalid value
                    }
                    // Apply the validated value
                    strncpy(params[i].value.string_value, str_value, sizeof(params[i].value.string_value) - 1);
                    params[i].value.string_value[sizeof(params[i].value.string_value) - 1] = '\0';
                    break;
                }
                default:
                    // Return error for unsupported types
                    return -1;
            }
            return 0; // Success
        }
    }
    return -1; // Parameter not found
}

// Retrieves a parameter by its key
int get_param(int key, config_param_t *out_param) {
    for (size_t i = 0; i < MAX_PARAMS; i++) {
        if (params[i].key == key) {
            *out_param = params[i];
            return 0;
        }
    }
    return -1; // Parameter not found
}

// Resets all parameters to their default values
void reset_params_to_defaults() {
    memcpy(params, default_params, sizeof(params));
}

// Saves current parameters to flash storage
void save_params_to_flash() {
    flash_storage_write(params, sizeof(params));
}

// Loads parameters from flash storage
void load_params_from_flash() {
    flash_storage_read(params, sizeof(params));
}

/*
 * Note: This configuration system assumes that default parameter values
 * are defined in a separate source file (`secret.c`) and linked at build time.
 */
