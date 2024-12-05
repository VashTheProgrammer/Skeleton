#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "command_sequence.h"

#define MAX_COMMANDS 32

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

void process_command_sequence(CommandNode *command_list, at_command_state_t *current_state, int *at_command_counter) {
    if (!command_list || !current_state || !at_command_counter) {
        printf("Error: Null pointer passed to process_command_sequence\n");
        return;
    }

    for (CommandNode *temp = command_list; temp != NULL; temp = temp->next) {
        if (temp->command.state == *current_state) {
            if (temp->command.state == STATE_SEND_RST) {
                gpio_put(atoi(temp->command.command), 0);
                sleep_ms(1);
                gpio_put(atoi(temp->command.command), 1);
                printf("%s\n", temp->command.message);
                *current_state = temp->command.next_state;
                break;
            }
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
