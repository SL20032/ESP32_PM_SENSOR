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
#define NVS_REQ_HOST_IP_0 "hostip0"
#define NVS_REQ_HOST_path_0 "hostpath0"
#define NVS_REQ_ACTIVATION_0 " reqact0"

esp_err_t write_str_to_nvs(char* nvs_key, char* nvs_data);
esp_err_t read_str_from_nvs(const char* nvs_key,char* nvs_data);
esp_err_t write_uint8_to_nvs(char* nvs_key, uint8_t nvs_data);
esp_err_t read_uint8_from_nvs(const char* nvs_key,uint8_t nvs_data);

void init_NVS(void);

#endif