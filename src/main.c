#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "uart.h"
#include "access_protocol.h"
#include "string.h"
#include "stdio.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "esp_log.h"

#include "cJSON.h"

#define LED_PIN GPIO_NUM_5
// kan variera på Nano ESP32
access_message_t msg;
access_status_t s;
static const char *TAG = "API";

#define MAX_HTTP_OUTPUT 512

static char response_buffer[MAX_HTTP_OUTPUT];
static int response_len = 0;

/* event handler connect wifi*/

static volatile bool wifi_connected = false;

void send_to_api(const char *json, const char *endpoint);

void wifi_init_simple(void);

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
  if (event_base == WIFI_EVENT &&
      event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    wifi_connected = false;

    wifi_event_sta_disconnected_t *disconn =
        (wifi_event_sta_disconnected_t *)event_data;

    ESP_LOGE(TAG, "WiFi failed, reason: %d", disconn->reason);

    esp_wifi_connect();
  }

  if (event_base == IP_EVENT &&
      event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

    wifi_connected = true;
  }
}

// void send_uid_to_api(const char *uid)
// {
//   cJSON *root = cJSON_CreateObject();

//   cJSON_AddStringToObject(root, "uid", uid);

//   char *json_string = cJSON_PrintUnformatted(root);

//   send_to_api(json_string);

//   free(json_string);
//   cJSON_Delete(root);
// }

void send_msg_to_api(access_message_t msg)
{
  cJSON *root = cJSON_CreateObject();
  char endp[15] = "";

  if (msg.command == ACCESS_CMD_UID)
  {

    cJSON_AddStringToObject(root, "uid", msg.data);
    strcpy(endp, "auth/uid");
  }
  if (msg.command == ACCESS_CMD_PIN)
  {

    cJSON_AddNumberToObject(root, "sid", msg.sid);
    cJSON_AddStringToObject(root, "pin", msg.data);
    strcpy(endp, "auth/pin");
  }

  char *json_string = cJSON_PrintUnformatted(root);
  if (strlen(endp) > 0)
  {
    send_to_api(json_string, endp);
  }
  free(json_string);
  cJSON_Delete(root);
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id)
  {
  case HTTP_EVENT_ON_DATA:

    if (response_len + evt->data_len < MAX_HTTP_OUTPUT)
    {
      memcpy(response_buffer + response_len,
             evt->data,
             evt->data_len);

      response_len += evt->data_len;
    }
    break;

  case HTTP_EVENT_ON_FINISH:
    response_buffer[response_len] = '\0'; // null-terminate
    cJSON *root = cJSON_Parse(response_buffer);
    cJSON *stat = cJSON_GetObjectItem(root, "status");
    cJSON *siditem = cJSON_GetObjectItem(root, "sid");
    // konvertera till uart och skicka tillbaka via tx1

    if (cJSON_IsNumber(siditem) && cJSON_IsString(stat))
    {
      uint16_t sid = (uint16_t)siditem->valueint;

      if (strcmp(stat->valuestring, "REQ_PIN") == 0)
      {
        access_send_pin_req(sid);
      }
      else if (strcmp(stat->valuestring, "OK") == 0)
      {
        access_send_ok(sid);
      }
      else if (strcmp(stat->valuestring, "PONG") == 0)
      {
        access_send_pong();
      }
    }
    break;

  case HTTP_EVENT_DISCONNECTED:
    response_len = 0; // reset
    break;

  default:
    break;
  }

  return ESP_OK;
}

void app_main(void)
{
  gpio_reset_pin(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

  gpio_set_level(LED_PIN, 1);
  vTaskDelay(500 / portTICK_PERIOD_MS);

  gpio_set_level(LED_PIN, 0);
  vTaskDelay(500 / portTICK_PERIOD_MS);

  wifi_init_simple();

  while (1)
  {

    if (uart_available() != 0)
    {

      s = access_read_message(&msg);

      if (wifi_connected)
      {
        send_msg_to_api(msg);

        uart_write_char('u');
        uart_write_char('i');
        uart_write_char('d');
        uart_write_char('o');
        uart_write_char('k');
        uart_write_char('\n');
      }
    }
  }
}

void wifi_init_simple(void)
{
  nvs_flash_init();
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_register(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      wifi_event_handler,
      NULL);

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = "AndroidAPB7D2",
          .password = "eree0104",
      },
  };

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();
  esp_wifi_connect();
}

void send_to_api(const char *json, const char *endpoint)
{

  char url[50];
  snprintf(url, sizeof(url),
           "http://192.168.43.34:8000/%s",
           endpoint);

  esp_http_client_config_t config = {
      .url = url, // ändra denna
      .method = HTTP_METHOD_POST,
      .event_handler = http_event_handler,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_http_client_set_header(client, "Content-Type", "application/json");

  esp_http_client_set_post_field(client, json, strlen(json));

  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK)
  {
    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP Status = %d", status);
  }
  else
  {
    ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
  }

  esp_http_client_cleanup(client);
}
