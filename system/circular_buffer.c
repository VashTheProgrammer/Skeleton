#include "circular_buffer.h"
#include <stdbool.h>

void circular_buffer_init(circular_buffer_t *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->full = false;
}

bool circular_buffer_enqueue(circular_buffer_t *cb, data_packet_t data) {
    if (cb->full) {
        return false; // Coda piena
    }
    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->full = (cb->head == cb->tail);
    return true;
}

bool circular_buffer_dequeue(circular_buffer_t *cb, data_packet_t *data) {
    if (circular_buffer_is_empty(cb)) {
        return false; // Coda vuota
    }
    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->full = false;
    return true;
}

bool circular_buffer_is_empty(circular_buffer_t *cb) {
    return (!cb->full && (cb->head == cb->tail));
}

bool circular_buffer_is_full(circular_buffer_t *cb) {
    return cb->full;
}