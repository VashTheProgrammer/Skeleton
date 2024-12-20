#include "config.h"

// WARNING: This file contains default configuration parameters and should be added to `.gitignore` 
// in real-world scenarios to avoid exposing sensitive information or application-specific defaults.

// Default parameter values
const config_param_t default_params[MAX_PARAMS] = {
    // Integer parameter with a validation range (0 to 100)
    // NOTE: For integer types, the `validation` field uses `int_range`, which defines a minimum and maximum value.
    {1, PARAM_TYPE_INT, .value.int_value = 42, .validation.int_range = {0, 100}},

    // String parameter without specific validation
    // NOTE: For string types, the `validation` field is empty (`{0}`), meaning there are no restrictions.
    {2, PARAM_TYPE_STRING, .value.string_value = "marrow", .validation = {0}},

    // Float parameter with a validation range (0.0 to 60.0)
    // NOTE: For float types, the `validation` field uses `float_range`, which defines a minimum and maximum float value.
    // Always ensure ranges for float are explicitly set, including `0.0`, to avoid logical errors.
    {3, PARAM_TYPE_FLOAT, .value.float_value = 3.14, .validation.float_range = {0.0, 60.0}},

    // Float parameter with a wider range (-100.0 to 100.0)
    // Demonstrates how negative and positive ranges can be set for float parameters.
    {4, PARAM_TYPE_FLOAT, .value.float_value = -3.14, .validation.float_range = {-100.0, 100.0}},

    // String parameter without validation
    // NOTE: As with the earlier string parameter, no specific constraints are applied here.
    {5, PARAM_TYPE_STRING, .value.string_value = "default", .validation = {0}},

    // Additional parameters can be added here
};
