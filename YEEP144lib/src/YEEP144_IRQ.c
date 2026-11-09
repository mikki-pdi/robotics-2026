/**
 * This file contains all of the IRQ handlers that are required for various parts
 * of the HAL and YEEP libraries to function. Unhandled IRQs will cause the system
 * to jump to the default handler, trapping it in an infinite loop
 */

#include "YEEP144_HCSR04.h"
#include "YEEP144_PushButton.h"
#include "YEEP144_PiSPI.h"
#include "stm32h563xx.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_dma.h"
#include "stm32h5xx_hal_spi.h"
#include "stm32h5xx_hal_uart.h"

extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern SPI_HandleTypeDef hspi4;

void YEEP144_BluetoothUART_MatchCallback(void);
void YEEP144_DebugUART_MatchCallback(void);

/** 
 * Handler for the SysTick interrupt
 * The HAL configures the SysTick to generate an interrupt every 1ms
 * HAL_IncTick needs to be called in here for the HAL to function properly
 */
void SysTick_Handler(void){
    HAL_IncTick();
}

/**
 * Handler for EXTI line 6. In the YEEP libraries this is used by the yellow pushbutton
 */
void EXTI6_IRQHandler(void){
    YEEP144_PushButton_YellowHandler();
}

/**
 * Handler for EXTI line 7. In the YEEP libraries this is used by the green pushbutton
 */
void EXTI7_IRQHandler(void){
    YEEP144_PushButton_GreenHandler();
}

/**
 * Handler for EXTI line 10. In the YEEP libraries this is used by ultrasonic sensors 1, 2, and 3
 */
void EXTI10_IRQHandler(void){
    YEEP144_Ultrasonic_EXTI10Handler();
}

/**
 * Handler for EXTI line 12. In the YEEP libraries this is used by ultrasonic sensor 4, and pushbutton 3 / red
 */
void EXTI12_IRQHandler(void){
    YEEP144_PushButton_RedHandler();
    YEEP144_Ultrasonic_EXTI12Handler();
}

/**
 * Handler for EXTI line 14. In the YEEP libraries this is used by the blue pushbutton
 */
void EXTI13_IRQHandler(void){
    YEEP144_PushButton_BlueHandler();
}

/**
 * Handler for GPDMA1 channel 0. In the YEEP libraries this is used by the BT UART reception
 * We just call the relevant HAL handler to let it deal with it
 */
void GPDMA1_CH0_IRQHandler(void){
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel0);

}

/**
 * Handler for GPDMA1 channel 0. In the YEEP libraries this is used by the BT UART reception
 * We just call the relevant HAL handler to let it deal with it
 */
void GPDMA1_CH1_IRQHandler(void){
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);

}

/**
 * Handler for SPI4 IRQ. We just call the relevant HAL handler to let it deal with it
 */
void SPI4_IRQHandler(void){
    HAL_SPI_IRQHandler(&hspi4);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi->Instance == SPI4){
        YEEP144_PiSPI_TxRxHandler();
    }
}

/**
 * Handler for USART2. In the YEEP libraries this is used by the BT UART.
 * We do our own checks for the character match interrupt as this is not handled
 * by the HAL. For any other interrupt we just let the HAL handle it
 */
void USART2_IRQHandler(void){
    uint32_t isrflags   = READ_REG(huart2.Instance->ISR);
    uint32_t cr1its     = READ_REG(huart2.Instance->CR1);

    // If the character match is enabled and character match flag raised
    if (((isrflags & USART_ISR_CMF) != 0U) && ((cr1its & USART_CR1_CMIE) != 0U)){
        __HAL_UART_CLEAR_FLAG(&huart2, UART_CLEAR_CMF); // Clear the character match flag
        HAL_DMA_Abort(huart2.hdmarx); // End the current DMA
        // Reset the state of the UART HAL
        huart2.RxState = HAL_UART_STATE_READY;
        huart2.ReceptionType = HAL_UART_RECEPTION_STANDARD;

        // Clear the idle line bit so the reception can be rearmed
        ATOMIC_CLEAR_BIT(huart2.Instance->CR1, USART_CR1_IDLEIE);

        // Call the match callback
        YEEP144_BluetoothUART_MatchCallback();
        return;
    }

    // If this is not the character match IRQ, let the HAL handle it
    HAL_UART_IRQHandler(&huart2);
}

/**
 * Handler for USART3. In the YEEP libraries this is used by the BT UART.
 * We do our own checks for the character match interrupt as this is not handled
 * by the HAL. For any other interrupt we just let the HAL handle it
 */
void USART3_IRQHandler(void){
    uint32_t isrflags   = READ_REG(huart3.Instance->ISR);
    uint32_t cr1its     = READ_REG(huart3.Instance->CR1);

    // If the character match is enabled and character match flag raised
    if (((isrflags & USART_ISR_CMF) != 0U) && ((cr1its & USART_CR1_CMIE) != 0U)){
        __HAL_UART_CLEAR_FLAG(&huart3, UART_CLEAR_CMF); // Clear the character match flag
        HAL_DMA_Abort(huart3.hdmarx); // End the current DMA
        // Reset the state of the UART HAL
        huart3.RxState = HAL_UART_STATE_READY;
        huart3.ReceptionType = HAL_UART_RECEPTION_STANDARD;

        // Clear the idle line bit so the reception can be rearmed
        ATOMIC_CLEAR_BIT(huart3.Instance->CR1, USART_CR1_IDLEIE);

        // Call the match callback
        YEEP144_DebugUART_MatchCallback();
        return;
    }

    // If this is not the character match IRQ, let the HAL handle it
    HAL_UART_IRQHandler(&huart3);
}

/**
 * These weakly defined callback functions are here so that the code always compiles
 * You shouldn't edit these functions, but instead implement your own with the exact same name
 * in one of your own files, like main.c
 */
__weak void YEEP144_BluetoothUART_MatchCallback(void){
    return;
}

__weak void YEEP144_DebugUART_MatchCallback(void){
    return;
}

/**
* These functions copied from STM's interrupt code to provide stubs for handling
* the internal Cortex Processor interrupts and exceptions
*/

/**
* This function handles Non maskable interrupt.
*/
void NMI_Handler(void){
    while (1){
        
    }
}

/**
* This function handles Hard fault interrupt.
*/
void HardFault_Handler(void){
    while (1){
        
    }
}

/**
* This function handles Memory management fault.
*/
void MemManage_Handler(void){
    while (1){
        
    }
}

/**
* This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void){
    while (1){
        
    }
}

/**
* This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void){
    while (1){
        
    }
}

/**
* This function handles System service call via SWI instruction.
*/
void SVC_Handler(void){

}

/**
* This function handles Debug monitor.
*/
void DebugMon_Handler(void){

}

/**
* This function handles Pendable request for system service.
*/
void PendSV_Handler(void){
    
}