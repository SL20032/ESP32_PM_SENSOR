#ifndef _WEB_SERWER_H_
#define _WEB_SERWER_H_

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include <stdbool.h>
#include "nvs.h"


#include <esp_http_server.h>



static httpd_handle_t start_webserver(bool is_config_mode);
static void stop_webserver(httpd_handle_t server);
static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void web_serwer_init(bool is_config_mode);
void serwer_data_update(uint16_t pm10_data, uint16_t pm25_data, uint16_t pm1_data);


#endif