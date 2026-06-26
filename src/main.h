#ifndef MAIN_H
#define MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "esp_log.h"

#include "cJSON.h"

#include "uart.h"
#include "access_protocol.h"

/* ===================== Defines ===================== */

#define LED_PIN GPIO_NUM_5

/* ===================== Types ===================== */

/* extern så de inte dupliceras i flera .c filer */
extern access_message_t msg;
extern access_status_t s;

/* ===================== Function declarations ===================== */

void app_main(void);

void wifi_init_simple(void);

void send_to_api(const char *json);

void send_uid_to_api(const char *uid);

#endif // MAIN_H