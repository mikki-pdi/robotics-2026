// YEEP Includes
#include "YEEP144.h"
#include "YEEP144_DCMotor.h"
#include "YEEP144_Pinout.h"

// HAL Includes
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_rcc.h"
#include "stm32h5xx_hal_tim.h"
#include "stm32h5xx_hal_lptim.h"
#include "stm32h5xx_hal_gpio.h"

// System Includes
#include <stdint.h>

// Structs for timer handles
// TIM3 used for all 4 DC motors
TIM_HandleTypeDef htim3;

// TIM3, LPTIM2, LPTIM3, LPTIM5 used for encoders
TIM_HandleTypeDef htim4;
LPTIM_HandleTypeDef hlptim2;
LPTIM_HandleTypeDef hlptim3;
LPTIM_HandleTypeDef hlptim5;

/** 
* Sets the specified motors speed by altering the duty cycle of the PWM signal
* This is done by setting the Capture and Compare Register for the relevant channel
* to a value between 0 and the Period of the timer where CCR = 0 is 0% duty cycle and
* CCR = Period is 100% duty cycle
*/
void YEEP144_DCMotor_SetSpeed(uint8_t motor, uint16_t speed){
    // Cap speed if out of bounds
    if(speed > DCMOTOR_MAX_SPEED){
        speed = DCMOTOR_MAX_SPEED;
    }

    // Update the Capture Compare Register (CCR) for the relevant motor channel
    switch(motor){
        case DCMOTOR1:
            TIM3->CCR1 = speed;
            break;
        case DCMOTOR2:
            TIM3->CCR2 = speed;
            break;
        case DCMOTOR3:
            TIM3->CCR3 = speed;
            break;
        case DCMOTOR4:
            TIM3->CCR4 = speed;
            break;
    }
}

/**
* Sets the two direction bits for the specified motor. There are 4 valid options
* for stopping, counter-clockwise movement, clockwise movement, and a short
* brake. There are #defines available for these states in YEEP144_DCMotor.h
*/
void YEEP144_DCMotor_SetDirection(uint8_t motor, uint8_t direction){
    // Check that the inputs are valid
    if(direction > 3 || motor > 4){
        return;
    }

    // Set the direction GPIO pins
    // Some bit manipulation is used to mask the relevant bit and shift it into the LSB position
    if(motor == DCMOTOR1){
        HAL_GPIO_WritePin(MOT1A_PORT, MOT1A_PIN, (direction>>1) & 1);
        HAL_GPIO_WritePin(MOT1B_PORT, MOT1B_PIN, direction & 1);
    }
    else if (motor == DCMOTOR2) {
        HAL_GPIO_WritePin(MOT2A_PORT, MOT2A_PIN, (direction>>1) & 1);
        HAL_GPIO_WritePin(MOT2B_PORT, MOT2B_PIN, direction & 1);
    }
    else if (motor == DCMOTOR3) {
        HAL_GPIO_WritePin(MOT3A_PORT, MOT3A_PIN, (direction>>1) & 1);
        HAL_GPIO_WritePin(MOT3B_PORT, MOT3B_PIN, direction & 1);
    }
    else if (motor == DCMOTOR4) {
        HAL_GPIO_WritePin(MOT4A_PORT, MOT4A_PIN, (direction>>1) & 1);
        HAL_GPIO_WritePin(MOT4B_PORT, MOT4B_PIN, direction & 1);
    }
}

/**
* Gets the current value of encoder 1
*/
uint16_t YEEP144_Encoder1_Get(void){
    return TIM4->CNT;
}

/**
* Gets the current value of encoder 2
*/
uint16_t YEEP144_Encoder2_Get(void){
    return LPTIM3->CNT;
}

/**
* Gets the current value of encoder 3
*/
uint16_t YEEP144_Encoder3_Get(void){
    return LPTIM5->CNT;
}

/**
* Gets the current value of encoder 4
*/
uint16_t YEEP144_Encoder4_Get(void){
    return LPTIM2->CNT;
}

/** 
* This function initialises all 4 DC motor driver channels
* They all share TIM3 for their PWM generation
* After initialisation the PWM will be set to 0%, and direction set to STOP
*
* This consumes TIM3 and the 4 GPIO it outputs on, as well as the 8 GPIO used for motor direction control 
*/
void YEEP144_DCMotor_Init(void){
    // Initialise TIM3

    // Create structures to store our timer configuration information
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    // Enable the clock input to TIM3
    __HAL_RCC_TIM3_CLK_ENABLE();

    // Setup our configuration data
    // For an input clock of 250MHz and a Period of 4095 these setting will produce
    // a PWM signal of ~61kHz which is within the range accepted by the motor driver

    // 4095 was chosen as the period as it maps 1-to-1 with the reading from a 12-bit ADC
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = DCMOTOR_MAX_SPEED;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    // Initialise the timer
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK){
    //Error_Handler();
    }

    // Configure master output
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK){
    //Error_Handler();
    }

    // Configure the Output Compare (OC) settings
    // This determines how the PWM signal is generated based on the count of our timer
    sConfigOC.OCMode = TIM_OCMODE_PWM1; // Edge-Aligned mode
    sConfigOC.Pulse = 0; // Default CCR value for our duty cycle
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH; // Non-inverted polarity
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE; // Fast mode is not required for this application

    // Configure the PWM channels
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    //Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK){
    //Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK){
    //Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK){
    //Error_Handler();
    }

    // Initialise GPIO pins used for PWM output
    // 3 of our PWM pins are on port C and 1 on port B
    // We must configure each port separately

    YEEP144_GPIO_InitAdv(MOT1PWM_PORT, MOT1PWM_PIN | MOT2PWM_PIN | MOT4PWM_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);
    YEEP144_GPIO_InitAdv(MOT3PWM_PORT, MOT3PWM_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM3);

    // Initialise GPIO for 2-bit direction control
    // The pins used are split across 3 ports
    // Each port must be configured separately

    YEEP144_GPIO_Init(MOT1A_PORT, MOT1A_PIN | MOT1B_PIN | MOT2A_PIN | MOT2B_PIN, GPIO_MODE_OUTPUT);
    YEEP144_GPIO_Init(MOT3A_PORT, MOT3A_PIN | MOT3B_PIN, GPIO_MODE_OUTPUT);
    YEEP144_GPIO_Init(MOT4A_PORT, MOT4A_PIN | MOT4B_PIN, GPIO_MODE_OUTPUT);

    // Set default outputs to stop
    YEEP144_DCMotor_SetDirection(DCMOTOR1, DCMOTOR_STOP);
    YEEP144_DCMotor_SetDirection(DCMOTOR2, DCMOTOR_STOP);
    YEEP144_DCMotor_SetDirection(DCMOTOR3, DCMOTOR_STOP);
    YEEP144_DCMotor_SetDirection(DCMOTOR4, DCMOTOR_STOP);

    // Start all the PWM channels
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

}

/**
* This initialises Encoder 1
* This consumes TIM4 and the two GPIO it takes input from
*/
void YEEP144_Encoder1_Init(void){
    // Initialise TIM4

    // Enable TIM4 clock
    __HAL_RCC_TIM4_CLK_ENABLE();

    // Create structs to hold config data
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 65535;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    // Configure encoder mode and input channels
    sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 0;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 0;

    // Initialise the timer
    if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK){
    //Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK){
    //Error_Handler();
    }

    // Initialise the GPIO used for the encoder input
    YEEP144_GPIO_InitAdv(ENC1A_PORT, ENC1A_PIN | ENC1B_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM4);

    // Start the encoder counting
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

}

/**
* This initialises Encoder 2
* This consumes LPTIM3 and the two GPIO it takes input from
*/
void YEEP144_Encoder2_Init(void){

    // Setup clock for LPTIM3

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM3;
    PeriphClkInitStruct.Lptim3ClockSelection = RCC_LPTIM3CLKSOURCE_PCLK3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    // Enable clock for LPTIM3
    __HAL_RCC_LPTIM3_CLK_ENABLE();

    // Configure LPTIM3 for encoder input mode
    hlptim3.Instance = LPTIM3;
    hlptim3.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim3.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
    hlptim3.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_RISING;
    hlptim3.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
    hlptim3.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim3.Init.Period = 65535;
    hlptim3.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim3.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
    hlptim3.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    hlptim3.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    hlptim3.Init.RepetitionCounter = 0;
    if (HAL_LPTIM_Init(&hlptim3) != HAL_OK){
    //Error_Handler();
    }

    // Initialise the GPIO used for the encoder input
    YEEP144_GPIO_InitAdv(ENC2A_PORT, ENC2A_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_LPTIM3);
    YEEP144_GPIO_InitAdv(ENC2B_PORT, ENC2B_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_LPTIM3);

    // Start the encoder counting
    HAL_LPTIM_Encoder_Start(&hlptim3);
}

/**
* This initialises Encoder 3
* This consumes LPTIM5 and the two GPIO it takes input from
*/
void YEEP144_Encoder3_Init(void){

    // Setup clock for LPTIM5

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM5;
    PeriphClkInitStruct.Lptim3ClockSelection = RCC_LPTIM3CLKSOURCE_PCLK3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    // Enable clock for LPTIM3
    __HAL_RCC_LPTIM5_CLK_ENABLE();

    // Configure LPTIM5 for encoder input mode
    hlptim5.Instance = LPTIM5;
    hlptim5.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim5.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
    hlptim5.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_RISING;
    hlptim5.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
    hlptim5.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim5.Init.Period = 65535;
    hlptim5.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim5.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
    hlptim5.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    hlptim5.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    hlptim5.Init.RepetitionCounter = 0;
    if (HAL_LPTIM_Init(&hlptim5) != HAL_OK){
    //Error_Handler();
    }

    // Initialise the GPIO used for the encoder input
    YEEP144_GPIO_InitAdv(ENC3A_PORT, ENC3A_PIN | ENC3B_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF13_LPTIM5);

    // Start the encoder counting
    HAL_LPTIM_Encoder_Start(&hlptim5);
}

/**
* This initialises Encoder 4
* This consumes LPTIM2 and the two GPIO it takes input from
*/
void YEEP144_Encoder4_Init(void){

    // Setup clock for LPTIM2

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM2;
    PeriphClkInitStruct.Lptim3ClockSelection = RCC_LPTIM3CLKSOURCE_PCLK3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        //Error_Handler();
    }

    // Enable clock for LPTIM2
    __HAL_RCC_LPTIM2_CLK_ENABLE();

    // Configure LPTIM2 for encoder input mode
    hlptim2.Instance = LPTIM2;
    hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
    hlptim2.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_RISING;
    hlptim2.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
    hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim2.Init.Period = 65535;
    hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
    hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    hlptim2.Init.RepetitionCounter = 0;
    if (HAL_LPTIM_Init(&hlptim2) != HAL_OK){
    //Error_Handler();
    }

    // Initialise the GPIO used for the encoder input
    YEEP144_GPIO_InitAdv(ENC4A_PORT, ENC4A_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF3_LPTIM2);
    YEEP144_GPIO_InitAdv(ENC4B_PORT, ENC4B_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF3_LPTIM2);

    // Start the encoder counting
    HAL_LPTIM_Encoder_Start(&hlptim2);
}