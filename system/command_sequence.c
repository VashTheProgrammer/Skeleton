#include "command_sequence.h"
#include <stdio.h>
#include <string.h>
#include "hardware/uart.h"

#define MAX_COMMANDS 10

static CommandNode command_nodes[MAX_COMMANDS];
static int command_count = 0;

CommandNode *create_command_node(at_command_state_t state, const char *cmd, const char *msg, at_command_state_t next_state) {
    if (command_count >= MAX_COMMANDS) {
        printf("Error: Maximum number of commands reached\n");
        return NULL;
    }
    CommandNode *node = &command_nodes[command_count++];
    node->command.state = state;
    strncpy(node->command.command, cmd, sizeof(node->command.command) - 1);
    node->command.command[sizeof(node->command.command) - 1] = '\0';
    node->command.message = msg;
    node->command.next_state = next_state;
    node->next = NULL;
    return node;
}

void append_command(CommandNode **head, CommandNode *new_node) {
    if (new_node == NULL) {
        printf("Error: Tried to append a NULL node\n");
        return;
    }
    if (*head == NULL) {
        *head = new_node;
    } else {
        CommandNode *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}

void initialize_command_sequence(CommandNode **command_list, iot_config_t *config) {
    if (config == NULL) {
        printf("Error: config is NULL\n");
        return;
    }

    append_command(command_list, create_command_node(STATE_SEND_AT_READY, "AT\r\n", "Inviando AT alla UART1", STATE_SEND_AT_GMR));

    append_command(command_list, create_command_node(STATE_SEND_AT_GMR, "AT+GMR\r\n", "Inviando AT+GMR alla UART1", STATE_SEND_AT_CWMODE));

    append_command(command_list, create_command_node(STATE_SEND_AT_CWMODE, "AT+CWMODE=1\r\n", "Inviando AT+CWMODE=1 alla UART1", STATE_SEND_AT_CWLAP_SSID));

    char connect_wifi_SSID[256];
    snprintf(connect_wifi_SSID, sizeof(connect_wifi_SSID), "AT+CWLAP=\"%s\"\r\n", config->wifi_ssid);
    append_command(command_list, create_command_node(STATE_SEND_AT_CWLAP_SSID, connect_wifi_SSID, "Inviando AT+CWLAP=\"Wi-Fi\" alla UART1", STATE_SEND_AT_WIFI_CONNECT));

    char connect_wifi_cmd[256];
    snprintf(connect_wifi_cmd, sizeof(connect_wifi_cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", config->wifi_ssid, config->wifi_password);
    append_command(command_list, create_command_node(STATE_SEND_AT_WIFI_CONNECT, connect_wifi_cmd, "Inviando AT+CWJAP alla UART1", STATE_SEND_AT_LIST_AP));

    append_command(command_list, create_command_node(STATE_SEND_AT_LIST_AP, "AT+CWLAP\r\n", "Inviando AT+CWLAP alla UART1", STATE_SEND_AT_IP));

    append_command(command_list, create_command_node(STATE_SEND_AT_IP, "AT+CIPSTA?\r\n", "Inviando AT+CIPSTA? alla UART1", STATE_IDLE));
}

void process_command_sequence(CommandNode *command_list, at_command_state_t *current_state, int *at_command_counter) {
    if (!command_list || !current_state || !at_command_counter) {
        printf("Error: Null pointer passed to process_command_sequence\n");
        return;
    }

    for (CommandNode *temp = command_list; temp != NULL; temp = temp->next) {
        if (temp->command.state == *current_state) {
            if (uart_is_writable(uart1)) {
                if (temp->command.command[0] != '\0') {
                    (*at_command_counter)++;
                    printf("%s, richiesta numero: %d\n", temp->command.message, *at_command_counter);
                    uart_puts(uart1, temp->command.command);
                } else {
                    printf("%s\n", temp->command.message);
                }
                *current_state = temp->command.next_state;
            } else {
                printf("Warning: UART1 non è pronta per scrivere\n");
            }
            break;
        }
    }
}
