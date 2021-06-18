#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include <esp_http_server.h>
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi.h"
#include "get_req.h"

static const int RX_BUF_SIZE = 1024;
char buffer123[100];
char buf[64];
char wynik[6];
char buf1[500] = "<html>"
    "<head>"
    "<style>"
    "h1 {text-align: left;}"
    "h2 {text-align: left;}"
    "h3 {text-align: center;}"
    "p {text-align: left;}"
    "</style>"
    "</head>"
    "<body style='background-color:gray;color:white;'>"
    "<h1>STAN JAKOSCI POWIETRZA</h1>"
    "<h2>Normy jakosci powietrza</h2>"
    "<p>wartosc nory dla pm10: 50/m3</p>"
    "<p>wartosc nory dla pm2.5: 40ug/m3</p>"
    "<h2>aktualne wartosci</h2>";
char buf2[500] = "<p>|</p>"
    "<p>|</p>"
    "<p>|</p>"
    "<p>|</p>"
    "<h3>Designed by Stanislaw Lisek</h3>"
    "</body>"
    "</html>";
char buf4[1000];
int indeks = 0;
int pm1 = 0;
int pm25 = 0;
int pm10 = 0;
int i1 = 0;
int i2 = 0;




#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

//====================USART=================================

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


//======================================HTTP========================================================
/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    i1 = 0;
    i2 = 0;
    while(!(buf1[i2] == 0))
    {
        buf4[i1] = buf1[i2];
        i1++;
        i2++;
        if (i2 == 500)
        {
            break;
        }
    }
    i2 = 0;
    while(!(buffer123[i2] == 0))
    {
        buf4[i1] = buffer123[i2];
        i1++;
        i2++;
        if (i2 == 300)
        {
            break;
        }
    }
    i2 = 0;
    while (!(buf2[i2] == 0))
    {
        buf4[i1] = buf2[i2];
        i1++;
        i2++;
        if (i2 == 1000)
        {
            break;
        }
    }    

    
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        //ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/main",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = buf4
};
    
   

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    //ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        //ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        return server;
    }

    //ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        //ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        //ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}




//=================================MAIN_APP=========================================================
void app_main(void)
{
    gpio_reset_pin(23);
    gpio_set_direction(23, GPIO_MODE_OUTPUT);
    gpio_reset_pin(22);
    gpio_set_direction(22, GPIO_MODE_OUTPUT);
    gpio_reset_pin(21);
    gpio_set_direction(21, GPIO_MODE_OUTPUT);
    init();

    static httpd_handle_t server = NULL;
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_sta(); 
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    start_webserver();




    while (1)
    {
        char*  http_req;
        uart_read_bytes(UART_NUM_1, buf, 64, 20 / portTICK_RATE_MS);
        indeks = 0;

        if(buf[0] != 0)
        {
            while(indeks < 63)
            {
                if (buf[indeks] == 0x42 && buf[indeks + 1] == 0x4d)
                {
                    wynik[0] = buf[indeks + 10]; //pm1
                    wynik[1] = buf[indeks + 11];
                    wynik[2] = buf[indeks + 12]; //pm25
                    wynik[3] = buf[indeks + 13];
                    wynik[4] = buf[indeks + 14]; //pm10
                    wynik[5] = buf[indeks + 15];
                    pm1 = (wynik[0]<<8) + wynik[1];
                    pm25 = (wynik[2]<<8) + wynik[3];
                    pm10 = (wynik[4]<<8) + wynik[5];
                    sprintf(buffer123,"<p>pm1.0: %dug/m3</p><p>pm2.5: %dug/m3</p><p>pm10: %dug/m3</p>",pm1,pm25,pm10);

                    http_req = malloc(100);
                    sprintf(http_req,"pass=12345678&pm1=%d&pm25=%d&pm10=%d",pm1,pm25,pm10);
                    http_rest_with_url(http_req);
                    free(http_req);

                    if (pm10 > 50)
                    {
                        gpio_set_level(23, 1);
                    }
                    else if(pm25 > 40)
                    {
                        gpio_set_level(23, 1);
                    }
                    else
                    {
                        gpio_set_level(23, 0);
                    }
                    
                    gpio_set_level(22, 1);
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    gpio_set_level(22, 0);
                    

                    break;
                }
                indeks++;
                
            }    

        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    
    
    
}
