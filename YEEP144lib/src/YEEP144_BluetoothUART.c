// YEEP144 includes
#include "YEEP144.h"
#include "YEEP144_BluetoothUART.h"
#include "YEEP144_Pinout.h"

// HAL includes
#include "stm32h563xx.h"
#include "stm32h5xx.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_cortex.h"
#include "stm32h5xx_hal_dma.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_rcc.h"
#include "stm32h5xx_hal_uart.h"
#include "stm32h5xx_hal_uart_ex.h"

// Std includes
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define UART_POLL_TIMEOUT 1000

// Structure for UART info
UART_HandleTypeDef huart2;

DMA_HandleTypeDef handle_GPDMA1_Channel0;

/**
* Initialises USART2 for use with the Bluetooth Serial addon board
*
* This consumes the USART2 peripheral and all of the UART pins on the comms headers
* as well as Channel 0 on GPDMA1
*/
void YEEP144_BluetoothUART_Init(void){

    // DMA clock and interrupt
    __HAL_RCC_GPDMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);

    // Initialise and start the clock

    // Structure to hold clock config
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2; // Configuring USART2
    PeriphClkInitStruct.Lptim3ClockSelection = RCC_USART2CLKSOURCE_PCLK1; // Select PCLK1
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    // Enable the clock
    __HAL_RCC_USART2_CLK_ENABLE();

    // Populate the UART struct with the configuration data
    // 115200 8-N-1 with flow control
    huart2.Instance = USART2;
    huart2.Init.BaudRate           = 115200;                    // 115200 Baud rate
    huart2.Init.WordLength         = UART_WORDLENGTH_8B;        // 8 bit
    huart2.Init.Parity             = UART_PARITY_NONE;          // None parity
    huart2.Init.StopBits           = UART_STOPBITS_1;           // 1 Stop bit
    huart2.Init.Mode               = UART_MODE_TX_RX;           // Full duplex
    huart2.Init.HwFlowCtl          = UART_HWCONTROL_RTS_CTS;    // Full flow control (RTS, CTS)
    huart2.Init.OverSampling       = UART_OVERSAMPLING_8;
    huart2.Init.ClockPrescaler     = UART_PRESCALER_DIV1;
    if(HAL_UART_Init(&huart2) != HAL_OK){

    }

    // Initialise GPIO pins
    YEEP144_GPIO_InitAdv(UART2_TX_PORT, UART2_RX_PIN | UART2_TX_PIN | UART2_CTS_PIN | UART2_RTS_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF7_USART2);

    // Enable USART IRQs
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    HAL_UARTEx_DisableFifoMode(&huart2);

    // Populate DMA init structure
    handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
    handle_GPDMA1_Channel0.Init.Request = GPDMA1_REQUEST_USART2_RX;
    handle_GPDMA1_Channel0.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel0.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_GPDMA1_Channel0.Init.SrcInc = DMA_SINC_FIXED;
    handle_GPDMA1_Channel0.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_GPDMA1_Channel0.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel0.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel0.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel0.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel0.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel0.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel0.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel0.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel0) != HAL_OK)
    {
      //Error_Handler();
    }

    __HAL_LINKDMA(&huart2, hdmarx, handle_GPDMA1_Channel0);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      //Error_Handler();
    }

    // Set default character match to carriage return
    YEEP144_BluetoothUART_SetCharMatch('\n');

    // Disable buffering on stdin. This prevents problems when using scanf with the UART redirection
    setvbuf(stdin, NULL, _IONBF, 0);
}

/**
 * Prints to the Bluetooth UART, regardless of what the default UART is set to.
 * Arguments and returns are the same as the default printf
 *
 * Internally this doesn't work using a stream like default printf, but uses a buffer, so the command may
 * fail if the buffer cannot be allocated, or the output may be truncated if expanding the format specifiers causes
 * the string to expand by more than 256 bytes
 */
int YEEP144_BluetoothUART_printf(const char * format, ...){
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
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, bufSize, UART_POLL_TIMEOUT);

    // Free the memory used
    free(buffer);

    // Return number of characters printed
    return bufSize;
}

/**
 * Enables the character match IRQ. When the configured character received an interrupt is fired.
 * The current transfer will be terminated and the YEEP144_BluetoothUART_MatchCallback will be called
 */
void YEEP144_BluetoothUART_EnableCharMatchIRQ(void){
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_CM);
}

/**
 * Disables the character match IRQ
 */
void YEEP144_BluetoothUART_DisableCharMatchIRQ(void){
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_CM);
}

/**
 * Set the character match IRQ character. When the configured character received an interrupt is fired.
 * The current transfer will be terminated and the YEEP144_BluetoothUART_MatchCallback will be called
 */
void YEEP144_BluetoothUART_SetCharMatch(char match){
    __HAL_UART_DISABLE(&huart2);
    MODIFY_REG(huart2.Instance->CR2, USART_CR2_ADD_Msk, match<< USART_CR2_ADD_Pos);
    __HAL_UART_ENABLE(&huart2);
}

/**
 * Sets the pre and post delimiters for the BT status messages
 * Setting both to null disables the status messages entirely
 */
void YEEP144_BluetoothUART_SetStatusDelimiter(char prefix, char postfix){

    // Save this register so we can restore character match IRQ if needed
    uint32_t config = READ_REG(huart2.Instance->CR1);

    // Disable the character match irq so it doesn't fire during this function
    YEEP144_BluetoothUART_DisableCharMatchIRQ();

    uint8_t rxBuf[10];
    uint16_t rxSize;

    // Enter command mode
    HAL_UART_Transmit(&huart2, "$$$", 3, UART_POLL_TIMEOUT);

    // Wait to receive data back from BT module
    HAL_UARTEx_ReceiveToIdle(&huart2, rxBuf, 10, &rxSize, UART_POLL_TIMEOUT);

    // Set delimiters
    char txBuf[8] = {0};
    if(prefix == '\0' && postfix == '\0'){
        sprintf(txBuf, "SO,,%c",'\r');
    }
    else if(prefix == '\0'){
        sprintf(txBuf, "SO,,%c%c", postfix, '\r');
    }
    else if(postfix == '\0'){
        sprintf(txBuf, "SO,%c,%c", prefix, '\r');
    }
    else{
        sprintf(txBuf, "SO,%c,%c%c", prefix, postfix, '\r');
    }
    HAL_UART_Transmit(&huart2, txBuf, sizeof(rxBuf), UART_POLL_TIMEOUT);

    // Wait to receive data back from BT module
    HAL_UARTEx_ReceiveToIdle(&huart2, rxBuf, 10, &rxSize, UART_POLL_TIMEOUT);
    
    // Reboot device to make changes
    HAL_UART_Transmit(&huart2, "R,1", 3, UART_POLL_TIMEOUT);

    // Wait to receive data back from BT module
    HAL_UARTEx_ReceiveToIdle(&huart2, rxBuf, 10, &rxSize, UART_POLL_TIMEOUT);

    // Re-enable the character match IRQ if it previously was
    if(config & USART_CR1_CMIE){
        YEEP144_BluetoothUART_EnableCharMatchIRQ();
    }
}

#if UARTSEL == BLUETOOTHUART
/**
* This function overrides a function from the stdio library, effectively redirecting
* output to the UART port. This means that printf can print directly to the UART.
*/
int __io_putchar(int ch){
    HAL_UART_Transmit(&huart2, (uint8_t *) &ch, 1, UART_POLL_TIMEOUT);
    return ch;
}

/**
* This function overrides a function from the stdio library, effectively redirecting
* input from the UART port. This means that scanf can read directly from the UART.
*/
int __io_getchar(void){
    uint8_t ch = 0;

    /* Clear the Overrun flag just before receiving the first character */
    __HAL_UART_CLEAR_OREFLAG(&huart2);

    /* Wait for reception of a character on the USART RX line and echo this
    * character on console */
    HAL_UART_Receive(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif