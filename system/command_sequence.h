/* command_sequence.h */
#ifndef COMMAND_SEQUENCE_H
#define COMMAND_SEQUENCE_H

#include "config.h"
typedef enum {
    STATE_SEND_AT_READY,
    STATE_SEND_AT_GMR,
    STATE_SEND_AT_CWMODE,
    STATE_SEND_AT_CWLAP_SSID,
    STATE_SEND_AT_WIFI_CONNECT,
    STATE_SEND_AT_LIST_AP,
    STATE_SEND_AT_IP,
    STATE_IDLE,
} at_command_state_t;

typedef struct {
    at_command_state_t state;
    char command[256];
    const char *message;
    at_command_state_t next_state;
} at_command_t;

typedef struct CommandNode {
    at_command_t command;
    struct CommandNode *next;
} CommandNode;

CommandNode *create_command_node(at_command_state_t state, const char *cmd, const char *msg, at_command_state_t next_state);
void append_command(CommandNode **head, CommandNode *new_node);
void initialize_command_sequence(CommandNode **command_list, iot_config_t *config);
void process_command_sequence(CommandNode *command_list, at_command_state_t *current_state, int *at_command_counter);

#endif // COMMAND_SEQUENCE_H
