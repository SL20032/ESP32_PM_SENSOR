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


#define ESP_MODE_PIN 18   //32  
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
/*
static const int RX_BUF_SIZE = 65;  //rx buffer must be larger than buffor that you read to

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)
*/
//====================USART=================================
/*
void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    return txBytes;
}

*/
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
            char*  http_req;
            esp_err_t err = USART_SeadData(&pm1,&pm25,&pm10);
            if (err == ESP_OK)
            {
                serwer_data_update(pm10,pm25,pm1);
            
                http_req = malloc(100);
                sprintf(http_req,"pass=12345678&pm1=%d&pm25=%d&pm10=%d",pm1,pm25,pm10);
                http_rest_with_url(http_req,"10.14.1.103","/main");
                free(http_req);  
            }
    
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
}
