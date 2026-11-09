#ifndef __YEEP144_DEBUGUART_H
#define __YEEP144_DEBUGUART_H

#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_uart.h"

// Structure for UART info
extern UART_HandleTypeDef huart3;

void YEEP144_DebugUART_Init(void);
int YEEP144_DebugUART_printf(const char * format, ...);

void YEEP144_DebugUART_EnableCharMatchIRQ(void);
void YEEP144_DebugUART_DisableCharMatchIRQ(void);
void YEEP144_DebugUART_SetCharMatch(char match);

#endif