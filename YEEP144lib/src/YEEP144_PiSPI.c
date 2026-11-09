#include "YEEP144_Pinout.h"
#include "stm32h563xx.h"
#include "stm32h5xx.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_cortex.h"
#include "stm32h5xx_hal_def.h"
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_rcc.h"
#include "stm32h5xx_hal_spi.h"

#include "YEEP144.h"
#include "YEEP144_PiSPI.h"
#include "stm32h5xx_hal_spi_ex.h"
#include <string.h>

void YEEP144_PiSPI_RxCallback(void);

SPI_HandleTypeDef hspi4;

uint8_t PiSPIRxBuf[PISPI_BUFFER_SIZE];

uint8_t PiSPITxBuf[PISPI_BUFFER_SIZE];
uint8_t PiSPITxBufInternal[PISPI_BUFFER_SIZE];

/**
 * Initialises SPI communications for use with an attached Raspberry Pi. In this configuration the Pi will act as the master
 * therefore the STM32 will be configured as a slave. This means that the master can send / ask for data at any time, and the SPI
 * peripheral must be ready to avoid under/overruns.
 */
void YEEP144_PiSPI_Init(void){

    // Setup and enable clock
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI4;
    PeriphClkInitStruct.Spi4ClockSelection = RCC_SPI4CLKSOURCE_HSI; // HSI = 64MHz clock
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    __HAL_RCC_SPI4_CLK_ENABLE();

    // Setup GPIO
    YEEP144_GPIO_InitAdv(SPI_MOSI_PORT, SPI_MOSI_PIN | SPI_MISO_PIN | SPI_SCK_PIN , GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF5_SPI4);
    YEEP144_GPIO_Init(SPI_CS_PORT, SPI_CS_PIN, GPIO_MODE_OUTPUT);
    YEEP144_PiSPI_ResetReady();

    // Enable interrupt
    HAL_NVIC_SetPriority(SPI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI4_IRQn);

    hspi4.Instance = SPI4;
    hspi4.Init.Mode = SPI_MODE_SLAVE; // Slave mode
    hspi4.Init.Direction = SPI_DIRECTION_2LINES; // Full duplex transmission
    hspi4.Init.DataSize = SPI_DATASIZE_8BIT; // Standard 8-bit word size
    hspi4.Init.CLKPolarity = SPI_POLARITY_LOW; // Clock polarity and phase set for SPI mode 0
    hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi4.Init.NSS = SPI_NSS_SOFT; // Chip Select managed by software - we assume we are always selected so the CS line can be repurposed for the ready signal
    hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB; // MSB first
    hspi4.Init.TIMode = SPI_TIMODE_DISABLE; // Not using TI mode
    hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; // Not using CRC
    hspi4.Init.CRCPolynomial = 0x7;
    hspi4.Init.NSSPMode = SPI_NSS_PULSE_DISABLE; // Chip select is held not pulsed
    hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW; // Chip select is active low
    hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_04DATA; // FIFO threshold is 4-bytes which is the maximum
    hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE; // Master configuration is defaults as we aren't master
    hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE; // Don't need to swap IO pins
    hspi4.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY; // We aren't using the hardware ready signal
    hspi4.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
    if (HAL_SPI_Init(&hspi4) != HAL_OK){
    //Error_Handler();
    }

    // Set up our interrupt to wait for start of transfer from the master
    HAL_SPI_TransmitReceive_IT(&hspi4, PiSPITxBufInternal, PiSPIRxBuf, PISPI_BUFFER_SIZE);

    // Set the ready flag as we are now ready to communicate
    YEEP144_PiSPI_SetReady();

}

/**
 * Resets the ready signal to ask the master to stop sending / requesting data
 */
void YEEP144_PiSPI_ResetReady(void){
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET);
}

/**
 * Sets the ready signal to tell the master we are ready to receive data
 */
void YEEP144_PiSPI_SetReady(void){
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);
}

/**
 * Loads the latest data into the SPI peripheral. Call this function every time you change PiSPITxBuf.
 */
void YEEP144_PiSPI_LoadTx(void){
    /**
     * As we cannot know when the master will initiate a transfer, we must always be ready to receive and send data
     * to avoid overrun when receiving, or underrun when sending. This means that the SPI peripheral must be active
     * with a transfer pending before the master begins a transfer by begining the clock and sending data, and 
     * this pending transfer must be kept active as much of the time as possible. This means that we always start a
     * transfer with HAL_SPI_TransmitReceive_IT at the end of SPI initialisation, and immediately after the previous
     * transfer has finished in the relevant ISR.
     *
     * When a transfer is started with HAL_SPI_TransmitReceive_IT, the first x bytes from the transmit buffer is 
     * pre-loaded into the SPI peripheral's internal TX FIFO, with x depending on the FIFO Threshold configuration,
     * to avoid any issues with data underrun. This means that if data in the transmit buffer is changed after a 
     * transfer is started, the first x bytes will remain from the state of the buffer when the transfer was started, 
     * leading to messages with a mix of old and new data.
     *
     * In order to avoid this, the transmit data is double buffered, so that the user code cannot change the data while
     * a transfer is in progress, and this function aborts the transfer, which flushes the FIFO, and copies the front
     * buffer into the back buffer, before restarting the transfer, which prevents the buffer changeover from also
     * occurring mid transaction.
     *
     * This means there is a small window each time this function is called, and shortly after the end of a transfer
     * where the SPI will not be active as the data is changed over. The ready signal should be reset during this period
     * which will lead to a small delay on the master side
     */

    // Wait for current transfer to end
    while(((SPI4->SR & SPI_SR_CTSIZE_Msk) >> SPI_SR_CTSIZE_Pos) != PISPI_BUFFER_SIZE){}

    // Clear the ready signal to tell the master we are no longer ready
    YEEP144_PiSPI_ResetReady();

    // Stop the current transfer which flushes the old data from the FIFO
    HAL_SPI_Abort_IT(&hspi4);

    // Copy the user facing buffer into the internal buffer
    memcpy(PiSPITxBufInternal, PiSPITxBuf, PISPI_BUFFER_SIZE);

    // Restart the transfer now the buffer has been updated
    HAL_SPI_TransmitReceive_IT(&hspi4, PiSPITxBufInternal, PiSPIRxBuf, PISPI_BUFFER_SIZE);

    // Set the ready signal to tell the master we are now ready to continue communication
    YEEP144_PiSPI_SetReady();
}

/**
 * This function should be called by HAL_SPI_TxRxCpltCallback in YEEP144_IRQ to handle the end of a transfer
 * It loads the latest data into the transmit buffer, reinitialises the SPI peripheral ready for the next transfer,
 * and calls the user callback
 */
void YEEP144_PiSPI_TxRxHandler(void){

    // Clear the ready signal as we are not ready to receive until the message has been handled
    YEEP144_PiSPI_ResetReady();

    // Copy the user facing buffer into the internal buffer
    memcpy(PiSPITxBufInternal, PiSPITxBuf, PISPI_BUFFER_SIZE);

    // Restart the transfer now the buffer has been updated
    HAL_SPI_TransmitReceive_IT(&hspi4, PiSPITxBufInternal, PiSPIRxBuf, PISPI_BUFFER_SIZE);

    // Call the user callback function - the user will need to restore the ready signal when done
    // for more messages to be received
    YEEP144_PiSPI_RxCallback();
}

/**
 * This is a weakly defined function for the RX Callback function.
 * It should be overriden by an identically named function in user code
 */
__weak void YEEP144_PiSPI_RxCallback(void){
    return;
}
