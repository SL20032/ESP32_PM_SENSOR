#include "web_serwer.h"

typedef struct {
    uint16_t pm10;
    uint16_t pm25;
    uint16_t pm1;  
} pm_data_t;

static const char *TAG = "WebSerwer";
bool mode_status = false;


pm_data_t serwer_data = {
    .pm10   =   0,
    .pm25   =   0,
    .pm1    =   0
};


void serwer_data_update(uint16_t pm10_data, uint16_t pm25_data, uint16_t pm1_data)
{
    serwer_data.pm10 = pm10_data;
    serwer_data.pm25 = pm25_data;
    serwer_data.pm1 = pm1_data;
}

void special_char_decoder(const char* input_data, char* output_data)
{
    int input_len = strlen(input_data);
    int indeks1 = 0;

    for(int indeks = 0; indeks < input_len; indeks++)
    {
        if (input_data[indeks] == '%')
        {
            if(input_data[indeks + 1] < 60)    output_data[indeks1] = (input_data[indeks + 1] - 48) * 16;
            else if(input_data[indeks + 1] > 59 && input_data[indeks + 1] < 92) output_data[indeks1] = (input_data[indeks + 1] - 55) * 16;
            else if(input_data[indeks + 1] > 91)    output_data[indeks1] = (input_data[indeks + 1] - 87) * 16;

            if(input_data[indeks + 2] < 60)    output_data[indeks1] = output_data[indeks1] + (input_data[indeks + 2] - 48);
            else if(input_data[indeks + 2] > 59 && input_data[indeks + 2] < 92) output_data[indeks1] = output_data[indeks1] + (input_data[indeks + 2] - 55);
            else if(input_data[indeks + 2] > 91)    output_data[indeks1] = output_data[indeks1] + (input_data[indeks + 2] - 87);
            indeks = indeks + 2;
        }   
        else
        {
            output_data[indeks1] = input_data[indeks];
        }
        indeks1++;
    }
}

static esp_err_t main_site_handler(httpd_req_t *req1)
{
    
    char* data_mem;
    data_mem = malloc(400);
    
    httpd_resp_sendstr_chunk(req1,
        "<html>"
        "<head>"
        "<style>"

        ".sensor_data {"
            "background-color: grey;"
            "color: white;"
            "/*border: 6px solid black;*/"
            "margin: 20px;"
            "padding: 30px;"
            "font-size: 20px;}"

        "header {"
            "background-color: grey;"
            "padding: 40px;"
            "text-align: center;"
            "font-size: 35px;"
            "color: white;}"

        "article {"
            "padding: 40px;"
            "background-color: #f1f1f1;"
            "height: 500px;}"

        "footer {"
            "padding: 40px;"
            "background-color: grey;"
            "text-align: center;"
            "color: white;"
            "height: 150px;}"

        "</style>"
        "</head>"
        "<body>"

        "<header>"
            "<h1>Smog Monitoring Station</h1>"
        "</header>"
    );

    sprintf(data_mem,"<article><div class='sensor_data'><p>PM1: %dug/m3</p></div><br><div class='sensor_data'><p>PM25: %dug/m3</p></div>"
                     "<br><div class='sensor_data'><p>PM10: %dug/m3</p></div></article>",serwer_data.pm1,serwer_data.pm25,serwer_data.pm10);

    
    httpd_resp_sendstr_chunk(req1,data_mem);

    httpd_resp_sendstr_chunk(req1,
        "<footer>"
        "<br>"
        "<br>"
        "<br>"
        "<br>"
        "<br>"
        "<h1>Design by Stanislaw Lisek   2021</h1>"
        "</footer>"
        "</body>"
        "</html>"
    );  
    
    //last chunk of response
    httpd_resp_send_chunk(req1, NULL, 0);

    /* After sending the HTTP response the old HTTP request
    * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req1, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;

    return ESP_OK;
}

static const httpd_uri_t main_pm_site = {
    .uri       = "/main",
    .method    = HTTP_GET,
    .handler   = main_site_handler,
    .user_ctx  = NULL
};


/* An HTTP GET handler */
static esp_err_t wifi_config_handler(httpd_req_t *req)
{
    char* req_buf;
    char* data_to_analize;
    char* decoded_data;
    decoded_data = malloc(100);
    data_to_analize = malloc(100);
    memset(decoded_data,0,100); 
    memset(data_to_analize,0,100);    

    //first chunk of response
    httpd_resp_sendstr_chunk(req,
    "<html>"
    "<body style='background-color:gray;color:white;'>"
    "<h1>SMOG sensor: Configuration Mode</h1>"
    "<h2>change wifi data and wireless communication options</h2>"
    "<br>"
    "<form>"
        "<p>WiFi options:</p>"
        "<label for='wifissid'>WiFi ssid:</label><br>"
        "<input type='text' id='wifissid' name='wifissid' value = 'myssid' size='50'><br>"
        "<label for='wifipass'>WiFi password:</label><br>"
        "<input type='text' id='wifipass' name='wifipass' value = 'pass' size='50'><br>"
        "<br>"
        "<p>Wireless communication options:</p>"
        "<input type ='checkbox' id='req_send_0_en' name='req0en' value='ok'"
        "<label for= 'req_send_0_en'>Enable wireless sending data chanell 0</label><br><br>"
        "<label for='reqhostip'>Wireless communication chanell 0 target ip:</label><br>"
        "<input type='text' id='reqhostip' name='reqhostip' size='50'><br>"
        "<label for='reqhostpath'>Wireless communication chanell 0 target path:</label><br>"
        "<input type='text' id='reqhostpath' name='reqhostpath' size='50'><br>"
        "<label for='reqhostpass'>Wireless communication chanell 0 target password:</label><br>"
        "<input type='text' id='reqhostpass' name='reqhostpass' size='50'><br><br>"
        "<input type='submit' value='send'>"
    "</form>"
    "</body>"
    "</html>");

   

    int req_len  = httpd_req_get_url_query_len(req) + 1;
    req_buf = malloc(req_len);
    httpd_req_get_url_query_str(req,req_buf,req_len);
    ESP_LOGI(TAG, "Found URL query => %s", req_buf);
    esp_err_t res = httpd_query_key_value(req_buf,"wifissid",data_to_analize,100);
    if (res == ESP_OK && data_to_analize[0] != 0)
    {
        special_char_decoder(data_to_analize,decoded_data);
        write_str_to_nvs(NVS_WIFI_SSID_KEY,decoded_data);
    }

    memset(data_to_analize,0,100); 
    memset(decoded_data,0,100); 
    res = ESP_FAIL;

    res = httpd_query_key_value(req_buf,"wifipass",data_to_analize,100);
    if (res == ESP_OK && data_to_analize[0] != 0)
    {   
        special_char_decoder(data_to_analize,decoded_data);
        write_str_to_nvs(NVS_WIFI_PASS_KEY,decoded_data);
    }

    memset(data_to_analize,0,100);
    memset(decoded_data,0,100); 
    res = ESP_FAIL;

    res = httpd_query_key_value(req_buf,"req0en",data_to_analize,100);
    if (res == ESP_OK && data_to_analize[0] != 0)
    {
        write_uint8_to_nvs(NVS_REQ_ACTIVATION_0,1);

        res = ESP_FAIL;
        memset(data_to_analize,0,100);
        memset(decoded_data,0,100); 
        res = httpd_query_key_value(req_buf,"reqhostip",data_to_analize,100);
        if (res == ESP_OK && data_to_analize[0] != 0)
        {
            special_char_decoder(data_to_analize,decoded_data);
            write_str_to_nvs(NVS_REQ_HOST_IP_0,decoded_data);
        }

        res = ESP_FAIL;
        memset(data_to_analize,0,100);
        memset(decoded_data,0,100); 
        res = httpd_query_key_value(req_buf,"reqhostpath",data_to_analize,100);
        if (res == ESP_OK && data_to_analize[0] != 0)
        {
            special_char_decoder(data_to_analize,decoded_data);
            write_str_to_nvs(NVS_REQ_HOST_PATH_0,decoded_data);
        }

        res = ESP_FAIL;
        memset(data_to_analize,0,100);
        memset(decoded_data,0,100); 
        res = httpd_query_key_value(req_buf,"reqhostpass",data_to_analize,100);
        if (res == ESP_OK && data_to_analize[0] != 0)
        {
            special_char_decoder(data_to_analize,decoded_data);
            write_str_to_nvs(NVS_REQ_PASS_0,decoded_data);
        }

    }
    else
    {
        write_uint8_to_nvs(NVS_REQ_ACTIVATION_0,0);
    }

    //last chunk of response
    httpd_resp_send_chunk(req, NULL, 0);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;

    free(req_buf);
    free(data_to_analize);
    free(decoded_data);

}

static const httpd_uri_t wifi_config = {
    .uri       = "/config",
    .method    = HTTP_GET,
    .handler   = wifi_config_handler,
    .user_ctx  = NULL
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

static httpd_handle_t start_webserver(bool is_config_mode)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        if (is_config_mode == true)
        {
            httpd_register_uri_handler(server, &wifi_config);
        }
        else
        {
            httpd_register_uri_handler(server, &main_pm_site);
        }

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
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
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver(mode_status);
    }
}

void web_serwer_init(bool is_config_mode)
{
    static httpd_handle_t server = NULL;
    
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    
    mode_status = is_config_mode;

    start_webserver(mode_status);
}