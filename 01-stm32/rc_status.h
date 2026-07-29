#ifndef RC_STATUS_H
#define RC_STATUS_H

#include "stm32f4xx_hal.h"

void RC_Status_Init(UART_HandleTypeDef *huart);
void RC_Status_OnUartRxComplete(UART_HandleTypeDef *huart);

#endif
