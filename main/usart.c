#include "usart.h"

void USART_Init(void) {
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

int USART_SendData(const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    return txBytes;
}


esp_err_t USART_SeadData(int *PM1,int *PM25,int *PM10)
{
    
    char buf[64];
    uart_read_bytes(UART_NUM_1, buf, 64, 20 / portTICK_RATE_MS);
    int i = 0;
    char wynik[6];
    *PM10 = 0;
    *PM1 = 0;
    *PM25 = 0;

    if(buf[0] != 0)
    {
        while(i < 63)
        {
            if (buf[i] == 0x42 && buf[i + 1] == 0x4d)
            {
                int calculated_checksum = 0;
                int give_checksum = 0;
                for (int indeks = i; indeks < i + 30; indeks++)
                {
                    calculated_checksum = calculated_checksum + buf[indeks + i];
                    calculated_checksum = calculated_checksum & 0xFFFF;
                }
                give_checksum = (buf[i + 30]<<8) + buf[i + 31];
                if (give_checksum == calculated_checksum)
                {
                    wynik[0] = buf[i + 10]; //pm1
                    wynik[1] = buf[i + 11];
                    wynik[2] = buf[i + 12]; //pm25
                    wynik[3] = buf[i + 13];
                    wynik[4] = buf[i + 14]; //pm10
                    wynik[5] = buf[i + 15];
                    *PM1 = (wynik[0]<<8) + wynik[1];
                    *PM25 = (wynik[2]<<8) + wynik[3];
                    *PM10 = (wynik[4]<<8) + wynik[5];
                    return ESP_OK;                
                }
                else
                {
                    return ESP_FAIL;
                }
            }
            i++;
        }    
    }
    return ESP_FAIL;
}