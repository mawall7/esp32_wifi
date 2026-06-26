#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define ACCESS_UART UART_NUM_1

void uart_init_access(void)
{
    uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(ACCESS_UART, &cfg);

    uart_set_pin(
        ACCESS_UART,
        GPIO_NUM_43, // TX
        GPIO_NUM_44, // RX
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE);

    uart_driver_install(
        ACCESS_UART,
        1024,
        0,
        0,
        NULL,
        0);
}

void uart_write_char(char c)
{
    uart_write_bytes(ACCESS_UART, &c, 1);
}

bool uart_read_char(char *c)
{
    return uart_read_bytes(
               ACCESS_UART,
               (uint8_t *)c,
               1,
               0) == 1;
}

uint16_t uart_available(void)
{
    size_t len;
    uart_get_buffered_data_len(
        ACCESS_UART,
        &len);

    return (uint16_t)len;
}