#include "YEEP144.h"
#include "stm32h563xx.h"
#include "stm32h5xx_hal.h"
#include "YEEP144_DebugUART.h"
#include "YEEP144_BluetoothUART.h"
#include "string.h"

//Variables
uint8_t RxData[32] = {0};

 //Callbacks
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
    HAL_UART_Receive_DMA(&huart2, RxData, 32);
}

void YEEP144_BluetoothUART_MatchCallback(){
    YEEP144_DebugUART_printf("Callback successfully triggered!\r\n");
    YEEP144_DebugUART_printf("RxData: %s", RxData);
    memset(RxData, '\0', 32);
    HAL_UART_Receive_DMA(&huart2, RxData, 32);
}

int main(void){
    //Function Initializations
    YEEP144_Init();
    YEEP144_DebugUART_Init();
    YEEP144_BluetoothUART_Init();

    //Testing printf
    YEEP144_DebugUART_printf("Program started!\r\n");

    //Start DMA reception
    HAL_UART_Receive_DMA(&huart2, RxData, 32);

    //Enable character match IRQ
    YEEP144_BluetoothUART_SetCharMatch('X');
    YEEP144_BluetoothUART_EnableCharMatchIRQ();

    while(1) {      
        HAL_Delay(1000);
    }
}