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
    data_mem = malloc(200);
    
    httpd_resp_sendstr_chunk(req1,
        "<html>"
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
        "<p>wartosc nory dla pm10: 50 ug/m3</p>"
        "<p>wartosc nory dla pm2.5: 40 ug/m3</p>"
        "<h2>aktualne wartosci</h2>"
    );

    sprintf(data_mem,"<p>pm1.0: %dug/m3</p><p>pm2.5: %dug/m3</p><p>pm10: %dug/m3</p>",serwer_data.pm1,serwer_data.pm25,serwer_data.pm10);
    
    httpd_resp_sendstr_chunk(req1,data_mem);

    httpd_resp_sendstr_chunk(req1,
        "<br>"
        "<br>"
        "<br>"
        "<br>"
        "<h3>Designed by Stanislaw Lisek</h3>"
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
    char* wifi_pass;
    char* wifi_ssid;
    char* master_pass;
    data_to_analize = malloc(100);
    wifi_pass = malloc(100);
    wifi_ssid = malloc(100);
    master_pass = malloc(100);
    memset(wifi_pass,0,100);
    memset(wifi_ssid,0,100);  
    memset(master_pass,0,100);    

    //first chunk of response
    httpd_resp_sendstr_chunk(req,
    "<html>"
    "<body style='background-color:gray;color:white;'>"
    "<h1>ROZDZIELNIA: Configuration Mode</h1>"
    "<h2>change wifi data and master password</h2>"
    "<br>"
    "<form>"
        "<label for='wifissid'>WiFi ssid:</label><br>"
        "<input type='text' id='wifissid' name='wifissid' value = 'myssid'><br>"
        "<label for='wifipass'>WiFi password:</label><br>"
        "<input type='password' id='wifipass' name='wifipass' value = 'pass'><br>"
        "<input type='submit' value='send'>"
    "</form>");

   

    int req_len  = httpd_req_get_url_query_len(req) + 1;
    req_buf = malloc(req_len);
    httpd_req_get_url_query_str(req,req_buf,req_len);
    ESP_LOGI(TAG, "Found URL query => %s", req_buf);
    esp_err_t res = httpd_query_key_value(req_buf,"wifissid",data_to_analize,100);
    if (res == ESP_OK && data_to_analize[0] != 0)
    {
        ESP_LOGI(TAG, "Found URL query key value => %s", data_to_analize);
        special_char_decoder(data_to_analize,wifi_ssid);


        ESP_LOGI(TAG, "Found URL query key value after decoding => %s", wifi_ssid);
        write_to_nvs(NVS_WIFI_SSID_KEY,wifi_ssid);
    }

    memset(data_to_analize,0,100); 



    esp_err_t res1 = httpd_query_key_value(req_buf,"wifipass",data_to_analize,100);
    if (res1 == ESP_OK && data_to_analize[0] != 0)
    {
        ESP_LOGI(TAG, "Found URL query key value => %s", data_to_analize);
        special_char_decoder(data_to_analize,wifi_pass);


        ESP_LOGI(TAG, "Found URL query key value after decoding => %s", wifi_pass);
        write_to_nvs(NVS_WIFI_PASS_KEY,wifi_pass);
    }

    read_from_nvs(NVS_WIFI_SSID_KEY,wifi_ssid);
    read_from_nvs(NVS_WIFI_PASS_KEY,wifi_pass);

    char* web_serwer_data;
    web_serwer_data = malloc(200);
    
    sprintf(web_serwer_data,"<br><p>Current WiFi SSID: %s</p><p>Current WiFi pass: %s</p>",wifi_ssid,wifi_pass);

    //data part of response
    httpd_resp_sendstr_chunk(req,
        web_serwer_data
    );

    free(web_serwer_data);

    httpd_resp_sendstr_chunk(req,
        "</body>"
        "</html>"
    );

    
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
    free(wifi_pass);
    free(wifi_ssid);
    free(master_pass);
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