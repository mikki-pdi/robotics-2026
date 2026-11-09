#ifndef __YEEP144_H
#define __YEEP144_H

#include "stm32h5xx_hal.h"

#define GPIO_MODE_OUTPUT GPIO_MODE_OUTPUT_PP

#define DEBUGUART 1
#define BLUETOOTHUART 2

#define UARTSEL DEBUGUART

// Public Function Prototypes
int YEEP144_Init(void);

void delayUS(uint32_t us);

void YEEP144_GPIO_Init(GPIO_TypeDef* port, uint32_t pins, uint32_t mode);
void YEEP144_GPIO_InitAdv(GPIO_TypeDef* port, uint32_t pins, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t alternate);

#endif