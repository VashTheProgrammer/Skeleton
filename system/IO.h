#ifndef IO_H
#define IO_H

#include "stm32g0xx_hal.h"
#include <string.h>
#include <assert.h>

/* Maximum number of GPIO elements per structure */
#define IO_MAX_ELEMENTS 10

/* Struct representing a GPIO port-pin pair */
typedef struct {
    GPIO_TypeDef *port;   /* GPIO port (e.g., GPIOA, GPIOB...) */
    uint16_t pin;         /* GPIO pin number (e.g., GPIO_PIN_5) */
} GPIO_PortPin_t;

/* Single GPIO element containing port-pin and its logical value */
typedef struct {
    GPIO_PortPin_t portpin; /* GPIO port-pin definition */
    uint8_t value;          /* Logical value (0=RESET, 1=SET) */
} IO_Element_t;

/* Structure containing an array of GPIO elements */
typedef struct {
    IO_Element_t list[IO_MAX_ELEMENTS]; /* Array of GPIO elements */
    uint8_t element_count;              /* Number of valid elements in the list */
} IO_t;

/**
 * @brief Macro to define a new static GPIO structure with given name.
 *
 * This macro creates:
 *   - A static IO_t structure named IO_<name>
 *   - An inline getter function IO_Get_<name>()
 *
 * Example Usage:
 *   IO_DEFINE(relays);
 */
#define IO_DEFINE(name) \
    static IO_t IO_##name = {0}; \
    static inline IO_t* IO_Get_##name(void) { return &IO_##name; }

/**
 * @brief Initialize a GPIO structure by setting its element count and clearing memory.
 *
 * @param name  Name of the GPIO structure (as defined with IO_DEFINE)
 * @param count Number of GPIO elements to initialize
 *
 * Example:
 *   IO_Init(relays, 3);
 */
#define IO_Init(name, count) do { \
    IO_t *io = IO_Get_##name(); \
    assert((count) <= IO_MAX_ELEMENTS); \
    io->element_count = (count); \
    memset(io->list, 0, sizeof(io->list)); \
} while(0)

/**
 * @brief Set a GPIO element (port-pin pair and value) in a given structure.
 *
 * @param name      Name of the GPIO structure (defined with IO_DEFINE)
 * @param idx       Index of the GPIO element to set (0-based)
 * @param pportpin  GPIO_PortPin_t structure (port & pin)
 * @param val       Logical value to set (0=RESET, 1=SET)
 *
 * Example:
 *   IO_SetElement(relays, 0, (GPIO_PortPin_t){GPIOA, GPIO_PIN_5}, 1);
 */
#define IO_SetElement(name, idx, pportpin, val) do { \
    IO_t *io = IO_Get_##name(); \
    assert((idx) < IO_MAX_ELEMENTS); \
    io->list[(idx)].portpin = (pportpin); \
    io->list[(idx)].value = (val); \
} while(0)

/**
 * @brief Call WriteAll function using automatically resolved structure pointer.
 *
 * @param name Name of the GPIO structure (defined with IO_DEFINE)
 *
 * Example:
 *   IO_WriteAll(relays);
 */
#define IO_WriteAll(name) IO_WriteAllFn(IO_Get_##name())

/**
 * @brief Call GetAll function using automatically resolved structure pointer.
 *
 * @param name Name of the GPIO structure (defined with IO_DEFINE)
 *
 * Example:
 *   IO_GetAll(relays);
 */
#define IO_GetAll(name) IO_GetAllFn(IO_Get_##name())

/* Function prototypes for IO operations (implemented in IO.c) */
void IO_WriteAllFn(IO_t *io);
void IO_GetAllFn(IO_t *io);

#endif /* IO_H */
