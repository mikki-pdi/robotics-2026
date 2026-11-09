#include "YEEP144.h"
#include "YEEP144_I2C.h"
#include "YEEP144_Pinout.h"

#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_i2c.h"
#include <stdint.h>

I2C_HandleTypeDef hi2c2;

void YEEP144_I2C_MuxSelect(uint8_t busNumber){
    uint8_t I2CBusSel;

    // If busNumber is greater than 3, deselect all busses
    if(busNumber > 3){
        I2CBusSel = 0;
    }
    else{ // Else convert to 1-hot to activate bus
        I2CBusSel = 1 << busNumber;
    }

    // Send byte to mux chip to select bus
    HAL_I2C_Master_Transmit(&hi2c2, I2C_MUX_ADDR, &I2CBusSel, 1, 1000);
}

void YEEP144_I2C_Init(void){
    // Initialise and start the clock

    // Structure to hold clock config
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2; // Configuring I2C2
    PeriphClkInitStruct.Lptim3ClockSelection = RCC_I2C2CLKSOURCE_PCLK1; // Select PCLK1
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    // Enable the clock
    __HAL_RCC_I2C2_CLK_ENABLE();

    // Configure the peripheral
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x60808CD3; // Magic number taken from MX for 100KHz - see Reference Manual RM0481, section 48.4.5

    // We are not operating as a target, so the rest are just defaults
    hi2c2.Init.OwnAddress1 = 0; 
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; 
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; 
    hi2c2.Init.OwnAddress2 = 0; 
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK; 
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE; 
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE; 
    if (HAL_I2C_Init(&hi2c2) != HAL_OK){
        //Error_Handler();
    }

    //Configure Analogue filter (Enabled, which is default)
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK){
        //Error_Handler();
    }

    //Configure Digital filter (Disabled, which is default)
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK){
        //Error_Handler();
    }

    // Initialise GPIO pins
    YEEP144_GPIO_InitAdv(I2C_SDA_PORT, I2C_SDA_PIN | I2C_SCL_PIN, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF4_I2C2);
}