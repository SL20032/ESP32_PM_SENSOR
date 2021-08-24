#ifndef _AP_WIFI_H_
#define _AP_WIFI_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SSID      "Esp32"
#define WIFI_PASS      "testtest123"
#define WIFI_CHANNEL   5
#define MAX_STA_CONN   4

void wifi_init_softap(void);

#endif