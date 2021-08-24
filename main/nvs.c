#include "nvs.h"

static const char *TAG = "NVS";

esp_err_t write_str_to_nvs(char* nvs_key, char* nvs_data)
{
  nvs_handle_t nvs_write_str_handle;
  //opening NVS handle
  esp_err_t ret = nvs_open("config_data", NVS_READWRITE, &nvs_write_str_handle);
  if (ret == ESP_OK)
  {
    ret = nvs_set_str(nvs_write_str_handle,nvs_key,nvs_data);
    ESP_LOGI(TAG,"NVS write: %s",nvs_data);
    nvs_close(nvs_write_str_handle);
    return ((ret != ESP_OK) ? ESP_FAIL : ESP_OK);
  }
  else
  {
    ESP_LOGI(TAG,"NVS ERR: Failed to open an NVS handle");
    nvs_close(nvs_write_str_handle);
    return ESP_FAIL;
  }
  
}

esp_err_t write_uint8_to_nvs(char* nvs_key, uint8_t nvs_data)
{
  nvs_handle_t nvs_write_uint8_handle;
  //opening NVS handle
  esp_err_t ret = nvs_open("config_data", NVS_READWRITE, &nvs_write_uint8_handle);
  if (ret == ESP_OK)
  {
    ret = nvs_set_u8(nvs_write_uint8_handle,nvs_key,nvs_data);
    ESP_LOGI(TAG,"NVS write: %d",nvs_data);
    nvs_close(nvs_write_uint8_handle);
    return ((ret != ESP_OK) ? ESP_FAIL : ESP_OK);
  }
  else
  {
    ESP_LOGI(TAG,"NVS ERR: Failed to open an NVS handle");
    nvs_close(nvs_write_uint8_handle);
    return ESP_FAIL;
  }
  
}

esp_err_t read_str_from_nvs(const char* nvs_key,char* nvs_data)
{
  nvs_handle_t nvs_read_str_handle;
  //opening NVS handle
  esp_err_t ret = nvs_open("config_data", NVS_READWRITE, &nvs_read_str_handle);
  if (ret == ESP_OK)
  {
    size_t nvs_len;
    ret = nvs_get_str(nvs_read_str_handle,nvs_key,nvs_data,&nvs_len);
    ESP_LOGI(TAG,"NVS read: %s",nvs_data);
    nvs_close(nvs_read_str_handle);
    return ((ret != ESP_OK) ? ESP_FAIL : ESP_OK);
  }
  else
  {
    ESP_LOGI(TAG,"NVS ERR: Failed to open an NVS handle");
    nvs_close(nvs_read_str_handle);
    return ESP_FAIL;
  }
}

esp_err_t read_uint8_from_nvs(const char* nvs_key,uint8_t nvs_data)
{
  nvs_handle_t nvs_read_uint8_handle;
  //opening NVS handle
  esp_err_t ret = nvs_open("config_data", NVS_READWRITE, &nvs_read_uint8_handle);
  if (ret == ESP_OK)
  {
    ret = nvs_get_u8(nvs_read_uint8_handle,nvs_key,&nvs_data);
    ESP_LOGI(TAG,"NVS read: %d",nvs_data);
    nvs_close(nvs_read_uint8_handle);
    return ((ret != ESP_OK) ? ESP_FAIL : ESP_OK);
  }
  else
  {
    ESP_LOGI(TAG,"NVS ERR: Failed to open an NVS handle");
    nvs_close(nvs_read_uint8_handle);
    return ESP_FAIL;
  }
}



void init_NVS(void)
{


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
}
