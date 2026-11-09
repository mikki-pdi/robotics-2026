#ifndef __YEEP144_I2C_H
#define __YEEP144_I2C_H

#include "stm32h5xx_hal.h"

#define I2C_MUX_ADDR 0xEE

extern I2C_HandleTypeDef hi2c2;

void YEEP144_I2C_Init(void);
void YEEP144_I2C_MuxSelect(uint8_t busNumber);

#endif