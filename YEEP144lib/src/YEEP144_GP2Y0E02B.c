#include "YEEP144_GP2Y0E02B.h"
#include "YEEP144_I2C.h"
#include "stm32h5xx_hal_i2c.h"

#include <math.h>

#define I2C_TIMEOUT 1000

float YEEP144_GP2Y0E02B_Read(void){
    uint8_t IRShift;
    uint8_t IRValues[2] ={0};
    
    HAL_I2C_Mem_Read(&hi2c2, GP2Y0E02B_I2C_ADDR, GP2Y0E02B_SHIFT_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &IRShift, 1, I2C_TIMEOUT);
    HAL_I2C_Mem_Read(&hi2c2, GP2Y0E02B_I2C_ADDR, GP2Y0E02B_DISTANCE_REG_ADDR, I2C_MEMADD_SIZE_8BIT, IRValues, 2, I2C_TIMEOUT);

    return ((IRValues[0]*16.0) + IRValues[1])/ 16.0 / pow(2.0, IRShift);

}