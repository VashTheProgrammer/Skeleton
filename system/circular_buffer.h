#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>

#define BUFFER_SIZE 128

typedef struct {
    char byte;
    int additional_data; // Puoi aggiungere ulteriori campi qui
} data_packet_t;

typedef struct {
    data_packet_t buffer[BUFFER_SIZE];
    int head;
    int tail;
    bool full;
} circular_buffer_t;

void circular_buffer_init(circular_buffer_t *cb);
bool circular_buffer_enqueue(circular_buffer_t *cb, data_packet_t data);
bool circular_buffer_dequeue(circular_buffer_t *cb, data_packet_t *data);
bool circular_buffer_is_empty(circular_buffer_t *cb);
bool circular_buffer_is_full(circular_buffer_t *cb);

#endif
