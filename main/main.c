#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>
#include "driver/gpio.h"
#include <stdbool.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "AP_wifi.h"
#include "web_serwer.h"
#include "nvs.h"
#include "Station_wifi.h"
#include "get_req.h"
#include "usart.h"

#include "driver/uart.h"
#include "freertos/event_groups.h"


#define ESP_MODE_PIN 32  
#define ESP_ONBOARD 2

char buffer123[100];
char buf[64];
char wynik[6];
int indeks = 0;
int i1 = 0;
int i2 = 0;
int pm1;
int pm25;
int pm10;
bool is_config_mode = false;

void Mode_Pin_Init(void)
{
    gpio_reset_pin(ESP_MODE_PIN);
    gpio_set_direction(ESP_MODE_PIN,GPIO_MODE_INPUT);
    gpio_set_pull_mode(ESP_MODE_PIN,GPIO_PULLUP_ONLY);
}

void onboard_led_init(void)
{
    gpio_reset_pin(ESP_ONBOARD);
    gpio_set_direction(ESP_ONBOARD,GPIO_MODE_OUTPUT);
}

//=================================MAIN_APP=========================================================
void app_main(void)
{
    USART_Init();
    onboard_led_init();    
    Mode_Pin_Init();
    init_NVS();
    vTaskDelay(1000 / portTICK_RATE_MS);
    if (gpio_get_level(ESP_MODE_PIN) == 0)
    {
        //config mode
        gpio_set_level(ESP_ONBOARD,1);
        wifi_init_softap();
        web_serwer_init(true);
        is_config_mode = true;
    }
    else
    {
        //standard mode
        gpio_set_level(ESP_ONBOARD,0);
        wifi_init_sta();
        web_serwer_init(false);
        is_config_mode = false;
    }
    
    while (1)
    {
        if (!(is_config_mode))       
        {
            esp_err_t err = USART_SeadData(&pm1,&pm25,&pm10);
            if (err == ESP_OK)
            {
                serwer_data_update(pm10,pm25,pm1);
                httpd_send_data(pm10,pm25,pm1);         
            }
    
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
}
