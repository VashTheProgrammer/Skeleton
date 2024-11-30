#ifndef IOT_CONFIG_H
#define IOT_CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char wifi_ssid[32];
    char wifi_password[32];
    char device_id[32];
    char username[128];
    char password[128];
} iot_config_t;

extern iot_config_t iot_config;

#endif // IOT_CONFIG_H
