#include "config.h"

// OCCHIO AL VALIDATION.FLOAT_RANGE !!!
// OCCHIO AL FATTO CHE SE METTI I LIMITI DEVI METTERE 0.0 (se usi i float)

// Valori di default
const config_param_t default_params[MAX_PARAMS] = {
    {1, PARAM_TYPE_INT, .value.int_value = 42, .validation.int_range = {0, 100}},
    {2, PARAM_TYPE_STRING, .value.string_value = "marrow", .validation = {0}},
    {3, PARAM_TYPE_FLOAT, .value.float_value = 3.14, .validation.float_range = {0.0, 60.0}},
    {4, PARAM_TYPE_FLOAT, .value.float_value = -3.14, .validation.float_range = {-100.0, 100.0}},
    {5, PARAM_TYPE_STRING, .value.string_value = "default", .validation = {0}},
    // Altri parametri
};