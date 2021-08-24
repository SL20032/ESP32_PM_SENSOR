#ifndef _USART_H_
#define _USART_H_

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdbool.h>
#include <sys/param.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>


#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

static const int RX_BUF_SIZE = 65;

void USART_Init(void);
int USART_SendData(const char* data);
esp_err_t USART_SeadData(int *PM1,int *PM25,int *PM10);



#endif