#include "YEEP144.h"
#include "stm32h563xx.h"
#include "stm32h5xx_hal.h"
#include "YEEP144_DebugUART.h"
#include "YEEP144_BluetoothUART.h"
#include "string.h"
#include "YEEP144_I2C.h"
#include "YEEP144_GP2Y0E02B.h"

//Variables

int main(void){
    //Function Initializations
    YEEP144_Init();
    YEEP144_DebugUART_Init();
    YEEP144_I2C_Init();

    //Init code
    YEEP144_DebugUART_printf("Program Start Success!");
    YEEP144_I2C_MuxSelect(0);
    //Loop
    while(1) {
        //Reading IR sensors
        float distance_IR0 = YEEP144_GP2Y0E02B_Read();
        YEEP144_DebugUART_printf("IR0(int): %d\r\n", (int)distance_IR0);
        YEEP144_DebugUART_printf("IR0(float): %f\r\n", distance_IR0);

        HAL_Delay(1000);
    }
}