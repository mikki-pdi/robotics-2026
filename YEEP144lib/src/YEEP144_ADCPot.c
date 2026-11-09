#include "YEEP144.h"
#include "YEEP144_Pinout.h"
#include "YEEP144_ADCPot.h"

#include "stm32h5xx_hal_adc.h"
#include "stm32h5xx_hal_gpio.h"

#define ADC_TIMEOUT 1000

ADC_HandleTypeDef hadc2;

/**
 * Initialise the potentiometers. This consumes ADC2, along with 2 GPIO
 */
void YEEP144_ADCPot_Init(void){

    // Setup and enable the ADC peripheral clock
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
    PeriphClkInitStruct.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_HSI; // HSI = 64MHz input clock
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
      //Error_Handler();
    }

    __HAL_RCC_ADC_CLK_ENABLE();

    // Configure and initialise the ADC peripheral

    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1; // No clock division = 64MHz
    hadc2.Init.Resolution = ADC_RESOLUTION_12B; // 12-bit resolution (maximum)
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc2.Init.LowPowerAutoWait = DISABLE;
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.NbrOfConversion = 2; // 2 channels in use
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START; // Conversion triggered by software
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc2.Init.DMAContinuousRequests = DISABLE; // No DMA
    hadc2.Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
    hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc2.Init.OversamplingMode = ENABLE; // Enable oversampling
    hadc2.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256; // 256x oversampling gets us 4 extra bits of resolution
    hadc2.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8; // Shift by 8 to get us back to 12-bit output
    hadc2.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
    hadc2.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
    if (HAL_ADC_Init(&hadc2) != HAL_OK){
    //Error_Handler();
    }

    // Initialise the GPIO used
    YEEP144_GPIO_Init(POT1_PORT, POT1_PIN | POT2_PIN, GPIO_MODE_ANALOG);
}

/**
 * Gets the current value of potentiometer 1.
 * Returns a 12-bit value in the range 0 - 4095.
 * This blocks until the conversion is complete, ~1ms
 */
uint16_t YEEP144_ADCPot_GetPot1(void){

    // Configure this channel to be converted next
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_2;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    // With a 64MHz clock and 256x oversampling, 247.5 cycles sample gives us a conversion time of ~1ms
    // or a maximum sample rate of ~1kHz
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK){
    //Error_Handler();
    }

    // Start the conversion
    HAL_ADC_Start(&hadc2);

    // Wait until the conversion is finished
    HAL_ADC_PollForConversion(&hadc2, ADC_TIMEOUT);

    // Grab the converted value now conversion is finished
    uint16_t adcval = HAL_ADC_GetValue(&hadc2);

    // Stop the ADC
    HAL_ADC_Stop(&hadc2);

    return adcval;
}

/**
 * Gets the current value of potentiometer 2.
 * Returns a 12-bit value in the range 0 - 4095.
 * This blocks until the conversion is complete, ~1ms
 */
uint16_t YEEP144_ADCPot_GetPot2(void){
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    // With a 64MHz clock and 256x oversampling, 247.5 cycles sample gives us a conversion time of ~1ms
    // or a maximum sample rate of ~1kHz
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK){
    //Error_Handler();
    }

    // Start the conversion
    HAL_ADC_Start(&hadc2);

    // Wait until the conversion is finished
    HAL_ADC_PollForConversion(&hadc2, ADC_TIMEOUT);

    // Grab the converted value now conversion is finished
    uint16_t adcval = HAL_ADC_GetValue(&hadc2);

    // Stop the ADC
    HAL_ADC_Stop(&hadc2);

    return adcval;
}