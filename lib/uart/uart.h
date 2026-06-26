#ifndef UART_ACCESS_H
#define UART_ACCESS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Initierar UART för access (UART_NUM_1)
 */
void uart_init_access(void);

/**
 * Skickar en enskild byte via UART
 */
void uart_write_char(char c);

/**
 * Läser en byte från UART.
 * Returnerar true om en byte lästes, annars false.
 */
bool uart_read_char(char *c);

/**
 * Returnerar antal tillgängliga bytes i RX-buffer
 */
uint16_t uart_available(void);

#endif // UART_ACCESS_H