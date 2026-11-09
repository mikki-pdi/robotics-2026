// YEEP144 includes
#include "YEEP144.h"
#include "YEEP144_DebugUART.h"
#include "YEEP144_Pinout.h"

// HAL includes
#include "stm32h563xx.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_uart.h"

// Std includes
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define UART_POLL_TIMEOUT 1000

// Structure for UART info
UART_HandleTypeDef huart3;

DMA_HandleTypeDef handle_GPDMA1_Channel1;

/**
* Initialises USART3 for use as a debug console output via the STLink
* Virtual COM Port (VCP)
*
* This consumes the USART3 peripheral and no YEEP GPIO as it is routed internally
* on the Nucleo board and is not broken out
*/
void YEEP144_DebugUART_Init(void){

    // Initialise and start the clock

    // DMA clock and interrupt
    __HAL_RCC_GPDMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);

    // Structure to hold clock config
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3; // Configuring USART3
    PeriphClkInitStruct.Lptim3ClockSelection = RCC_USART3CLKSOURCE_PCLK1; // Select PCLK1
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    // Enable the clock
    __HAL_RCC_USART3_CLK_ENABLE();

    // Populate the UART struct with the configuration data
    // 115200 8-N-1
    huart3.Instance = USART3;
    huart3.Init.BaudRate           = 115200;                // 115200 Baud rate
    huart3.Init.WordLength         = UART_WORDLENGTH_8B;    // 8 bit
    huart3.Init.Parity             = UART_PARITY_NONE;      // None parity
    huart3.Init.StopBits           = UART_STOPBITS_1;       // 1 Stop bit
    huart3.Init.Mode               = UART_MODE_TX_RX;       // Full duplex
    huart3.Init.HwFlowCtl          = UART_HWCONTROL_NONE;   // No hardware flow control (RTS, CTS)
    huart3.Init.OverSampling       = UART_OVERSAMPLING_8;
    huart3.Init.ClockPrescaler     = UART_PRESCALER_DIV1;
    if(HAL_UART_Init(&huart3) != HAL_OK){

    }

    // Initialise GPIO pins
    YEEP144_GPIO_InitAdv(UART3_TX_PORT, UART3_RX_PIN | UART3_TX_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF7_USART3);


    // Enable USART IRQs
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    HAL_UARTEx_DisableFifoMode(&huart3);

    // Populate DMA init structure
    handle_GPDMA1_Channel1.Instance = GPDMA1_Channel1;
    handle_GPDMA1_Channel1.Init.Request = GPDMA1_REQUEST_USART3_RX;
    handle_GPDMA1_Channel1.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_GPDMA1_Channel1.Init.SrcInc = DMA_SINC_FIXED;
    handle_GPDMA1_Channel1.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_GPDMA1_Channel1.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel1.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel1.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel1.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel1.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel1.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel1.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel1.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel1) != HAL_OK)
    {
      //Error_Handler();
    }

    __HAL_LINKDMA(&huart3, hdmarx, handle_GPDMA1_Channel1);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      //Error_Handler();
    }

    // Set default character match to carriage return
    YEEP144_DebugUART_SetCharMatch('\r');

    // Disable buffering on stdin. This prevents problems when using scanf with the UART redirection
    setvbuf(stdin, NULL, _IONBF, 0);
}

/**
 * Prints to the Debug UART, regardless of what the default UART is set to.
 * Arguments and returns are the same as the default printf
 *
 * Internally this doesn't work using a stream like default printf, but uses a buffer, so the command may
 * fail if the buffer cannot be allocated, or the output may be truncated if expanding the format specifiers causes
 * the string to expand by more than 256 bytes
 */
int YEEP144_DebugUART_printf(const char * format, ...){
    // Calculate how long the input string is, plus extra space for substitition in the format specifiers
    size_t bufSize = strlen(format) + 256;

    // Try to allocate that buffer
    char* buffer = malloc(bufSize);

    // If the buffer cannot be allocated, return -1
    if (buffer == NULL) return -1;

    // Get the pointer to the variadic args
    va_list args;
    va_start (args, format);

    // Create our final string in the buffer using vsnprintf, passing through our argument pointer
    vsnprintf(buffer, bufSize, format, args);

    // Calulate the final size of the string
    bufSize = strlen(buffer);

    // Transmit over UART
    HAL_UART_Transmit(&huart3, (uint8_t*)buffer, bufSize, UART_POLL_TIMEOUT);

    // Free the memory used
    free(buffer);

    // Return number of characters printed
    return bufSize;
}

/**
 * Enables the character match IRQ. When the configured character received an interrupt is fired.
 * The current transfer will be terminated and the YEEP144_DebugUART_MatchCallback will be called
 */
void YEEP144_DebugUART_EnableCharMatchIRQ(void){
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_CM);
}

/**
 * Disables the character match IRQ
 */
void YEEP144_DebugUART_DisableCharMatchIRQ(void){
    __HAL_UART_DISABLE_IT(&huart3, UART_IT_CM);
}

/**
 * Set the character match IRQ character. When the configured character received an interrupt is fired.
 * The current transfer will be terminated and the YEEP144_DebugUART_MatchCallback will be called
 */
void YEEP144_DebugUART_SetCharMatch(char match){
    __HAL_UART_DISABLE(&huart3);
    MODIFY_REG(huart3.Instance->CR2, USART_CR2_ADD_Msk, match<< USART_CR2_ADD_Pos);
    __HAL_UART_ENABLE(&huart3);
}

#if UARTSEL == DEBUG
/**
* This function overrides a function from the stdio library, effectively redirecting
* output to the UART port. This means that printf can print directly to the UART.
*/
int __io_putchar(int ch){
    HAL_UART_Transmit(&huart3, (uint8_t *) &ch, 1, UART_POLL_TIMEOUT);
    return ch;
}

/**
* This function overrides a function from the stdio library, effectively redirecting
* input from the UART port. This means that scanf can read directly from the UART.
*/
int __io_getchar(void){
    uint8_t ch = 0;

    /* Clear the Overrun flag just before receiving the first character */
    __HAL_UART_CLEAR_OREFLAG(&huart3);

    /* Wait for reception of a character on the USART RX line and echo this
    * character on console */
    HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif