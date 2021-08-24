#ifndef _NVS_H_
#define _NVS_H_


#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include <stdbool.h>

#define NVS_WIFI_PASS_KEY "wifipass"
#define NVS_WIFI_SSID_KEY "wifissid"
#define NVS_MASTER_PASS_KEY "masterpass"

esp_err_t write_to_nvs(char* nvs_key, char* nvs_data);
esp_err_t read_from_nvs(const char* nvs_key,char* nvs_data);
void init_NVS(void);

#endif