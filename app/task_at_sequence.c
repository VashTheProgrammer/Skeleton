#include "task_led.h"
#include "circular_buffer.h"
#include "driver_led.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "initcalls.h"
#include "command_sequence.h"
#include "config.h"

#include "hardware_config.h"

// Set initial state
CommandNode *command_list = NULL;
at_command_state_t current_state = STATE_SEND_RST; // primo stato valido
int at_command_counter = 0;

void initialize_command_sequence(CommandNode **command_list, iot_config_t *config) {
    if (config == NULL) {
        printf("Error: config is NULL\n");
        return;
    }

    char esp32_gpio_rst[8];
    snprintf(esp32_gpio_rst, sizeof(esp32_gpio_rst), "%d", hw_config->esp32_rst_pin);
    append_command(command_list, create_command_node(STATE_SEND_RST, esp32_gpio_rst, "Reset AT host device", STATE_SEND_AT_READY));

    append_command(command_list, create_command_node(STATE_SEND_AT_READY, "AT\r\n", "Inviando AT alla UART1", STATE_SEND_AT_GMR));

    append_command(command_list, create_command_node(STATE_SEND_AT_GMR, "AT+GMR\r\n", "Inviando AT+GMR alla UART1", STATE_SEND_AT_CWMODE));

    append_command(command_list, create_command_node(STATE_SEND_AT_CWMODE, "AT+CWMODE=1\r\n", "Inviando AT+CWMODE=1 alla UART1", STATE_SEND_AT_CWLAP_SSID));

    char connect_wifi_SSID[128];
    snprintf(connect_wifi_SSID, sizeof(connect_wifi_SSID), "AT+CWLAP=\"%s\"\r\n", config->wifi_ssid);
    append_command(command_list, create_command_node(STATE_SEND_AT_CWLAP_SSID, connect_wifi_SSID, "Inviando AT+CWLAP=\"Wi-Fi\" alla UART1", STATE_SEND_AT_WIFI_CONNECT));

    char connect_wifi_cmd[128];
    snprintf(connect_wifi_cmd, sizeof(connect_wifi_cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", config->wifi_ssid, config->wifi_password);
    append_command(command_list, create_command_node(STATE_SEND_AT_WIFI_CONNECT, connect_wifi_cmd, "Inviando AT+CWJAP alla UART1", STATE_SEND_AT_LIST_AP));

    append_command(command_list, create_command_node(STATE_SEND_AT_LIST_AP, "AT+CWLAP\r\n", "Inviando AT+CWLAP alla UART1", STATE_SEND_AT_IP));

    append_command(command_list, create_command_node(STATE_SEND_AT_IP, "AT+CIPSTA?\r\n", "Inviando AT+CIPSTA? alla UART1", STATE_MQTT_CONFIG));

    char connect_azure_cfg[512];
    // AT+MQTTUSERCFG=0,1,"DeviceID","IoTHubHostname/DeviceID/?api-version=2018-06-30","SASToken",0,60
    snprintf(connect_azure_cfg, sizeof(connect_azure_cfg), "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,60\r\n", config->device_id, config->username, config->sas);
    append_command(command_list, create_command_node(STATE_MQTT_CONFIG, connect_azure_cfg, "Inviando AT+MQTTUSERCFG alla UART1", STATE_MQTT_CON));

    char connect_azure_con[128];
    // AT+MQTTCONN=0,"IoTHubHostname",8883,1
    snprintf(connect_azure_con, sizeof(connect_azure_con), "AT+MQTTCONN=0,\"%s\",8883,1\r\n", config->hostname);
    append_command(command_list, create_command_node(STATE_MQTT_CON, connect_azure_con, "Inviando AT+MQTTCONN alla UART1", STATE_IDLE));

}

void task_at_sequence_init(void) {

    gpio_init(hw_config->esp32_rst_pin);
    gpio_set_dir(hw_config->esp32_rst_pin, GPIO_OUT);  

    // Initialize command sequence
    initialize_command_sequence(&command_list, &iot_config);
}
REGISTER_INITCALL(task_at_sequence_init);


void task_at_sequence(void){
    process_command_sequence(command_list, &current_state, &at_command_counter);
}