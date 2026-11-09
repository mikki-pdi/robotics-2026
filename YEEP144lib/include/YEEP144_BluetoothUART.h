#ifndef __YEEP144_BLUETOOTHUART_H
#define __YEEP144_BLUETOOTHUART_H

#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_uart.h"

// Structure for UART info
extern UART_HandleTypeDef huart2;

void YEEP144_BluetoothUART_Init(void);
int YEEP144_BluetoothUART_printf(const char * format, ...);

void YEEP144_BluetoothUART_EnableCharMatchIRQ(void);
void YEEP144_BluetoothUART_DisableCharMatchIRQ(void);
void YEEP144_BluetoothUART_SetCharMatch(char match);

void YEEP144_BluetoothUART_SetStatusDelimiter(char prefix, char postfix);

#endif