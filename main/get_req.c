#include "get_req.h"

static const char *TAG = "HTTP_CLIENT";

extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                }
                output_len = 0;
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

void http_rest_with_url(char* buf, char* host_ip, char* host_path)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    gpio_set_level(12,1);
   
    esp_http_client_config_t config = {
        .host = host_ip,
        .path = host_path,
        .query = buf,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    // GET
    printf("-start-");
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("-ok-");
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
               
    } else {
        gpio_set_level(13,1);
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        printf("-non-");
    }
    ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
    printf("-end-");

    //zakończ połączenie z serwerem
    esp_http_client_cleanup(client);
}


void httpd_send_data(int pm10, int pm25, int pm1)
{
    uint8_t en_data = 0;
    read_uint8_from_nvs(NVS_REQ_ACTIVATION_0,&en_data);
    if (en_data == 1)
    {
        char*  httpd_req;
        char*  host_ip;
        char*  host_path;
        char*  host_pass;
        
        httpd_req = malloc(200);
        memset(httpd_req,0,200);

        host_pass = malloc(100);
        memset(host_pass,0,100);

        read_str_from_nvs(NVS_REQ_PASS_0,host_pass);
        sprintf(httpd_req,"pass=%s&pm1=%d&pm25=%d&pm10=%d",(char*)host_pass,pm1,pm25,pm10);
        free(host_pass);

        host_ip = malloc(100);
        memset(host_ip,0,100);

        host_path = malloc(100);
        memset(host_path,0,100);

        read_str_from_nvs(NVS_REQ_HOST_IP_0,host_ip);
        read_str_from_nvs(NVS_REQ_HOST_PATH_0,host_path);
        http_rest_with_url(httpd_req,(char*)host_ip,(char*)host_path);
        
        free(httpd_req);
        free(host_ip);
        free(host_path);
    }
    
    
    
    
    
    
    
}




