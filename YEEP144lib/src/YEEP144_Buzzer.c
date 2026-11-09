#include "YEEP144.h"
#include "YEEP144_Pinout.h"
#include "YEEP144_Buzzer.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_tim.h"

#include <math.h>
#include <stdlib.h>

TIM_HandleTypeDef htim17buzzer;

/**
 * Initialises the buzzer. This consumes timer 17 and 1 GPIO. This resource is shared with servo 4 - only 1 should be initialised
 * Ensure DIP switch SW5 is correctly set to enable the buzzer output. Set both to off when using servo 4 to avoid hearing
 * the servo PWM signal through the buzzer
 */
void YEEP144_Buzzer_Init(void){
    // Initialise TIM17

    // Create structures to store our timer configuration information
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    // Enable the clock input to TIM3
    __HAL_RCC_TIM17_CLK_ENABLE();

    // Setup our configuration data
    // Prescaler is set so that 1 timer tick = 1us for 250MHz input clock, as the frequency calculation assumes this
    htim17buzzer.Instance = TIM17;
    htim17buzzer.Init.Prescaler = 249;
    htim17buzzer.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim17buzzer.Init.Period = 65535;
    htim17buzzer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim17buzzer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    // Initialise the timer
    if (HAL_TIM_PWM_Init(&htim17buzzer) != HAL_OK){
    //Error_Handler();
    }

    // Configure the Output Compare (OC) settings
    // This determines how the PWM signal is generated based on the count of our timer
    sConfigOC.OCMode = TIM_OCMODE_PWM1; // Edge-Aligned mode
    sConfigOC.Pulse = 0; // Default CCR value for our duty cycle
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH; // Non-inverted polarity
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    // Configure the PWM channels
    if (HAL_TIM_PWM_ConfigChannel(&htim17buzzer, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    //Error_Handler();
    }

    // Disable the break dead time feature as its not needed
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim17buzzer, &sBreakDeadTimeConfig) != HAL_OK){
        //Error_Handler();
    }

    // Setup the GPIO output from the timer
    YEEP144_GPIO_InitAdv(BUZZER_PORT, BUZZER_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF1_TIM17);

    // Start PWM generation
    HAL_TIM_PWM_Start(&htim17buzzer, TIM_CHANNEL_1);

    // Default to no tone
    YEEP144_Buzzer_NoTone();
}

/**
 * Sound the buzzer at the chosen frequency (Hz) for the alloted duration (ms)
 * This is blocking as it uses HAL_Delay to wait the alloted time
 */
void YEEP144_Buzzer_Tone(uint32_t freq, uint32_t duration){
    YEEP144_Buzzer_GoTone(freq); // Start playing the tone
    HAL_Delay(duration); // Wait for tone duration
    YEEP144_Buzzer_NoTone(); // Cease playing the tone
}

/**
 * Sound the buzzer at the chosen frequency (Hz) continuously until stopped by NoTone
 * This is non-blocking, so you can go do other things while the tone is playing
 */
void YEEP144_Buzzer_GoTone(uint32_t freq){
    if(freq != 0){
        TIM17->ARR = (1000000UL / freq) - 1; // Set PWM frequency
        TIM17->CCR1 = (TIM17->ARR >> 1); // Set the duty cycle to 50% by setting Capture Compare Register to half the frequency
    }
}

/**
 * Stops the buzzer from sounding by setting the duty cycle to 0%
 */
void YEEP144_Buzzer_NoTone(void){
    TIM17->CCR1 = 0; // Set duty cycle to 0%
}


/**
 * Buzzer rendition of Daisy Bell taken from:
 * https://github.com/ayoungblood/daisy-bell/blob/master/arduino/daisy_basic/daisy_basic.ino
 */
#define A440 440 // frequency constant
#define QUARTER 180 // quarter note length in ms

const uint8_t DAISY_P[] = {
        74, 71, 67, 62, 64, 66, 67, 64, 67, 62,
        69, 74, 71, 67, 64, 66, 67, 69, 71, 69,
        71, 72, 71, 69, 74, 71, 69, 67, 69, 71, 67, 64, 67, 64, 62,
        62, 67, 71, 69, 67, 71, 69, 71, 72, 74, 71, 67, 69, 62, 67
};
const uint8_t DAISY_T[] = {
        3,  3,  3,  3,  1,  1,  1,  2,  1,  6,
        3,  3,  3,  3,  1,  1,  1,  2,  1,  6,
        1,  1,  1,  1,  2,  1,  1,  4,  1,  2,  1,  2,  1,  1,  5,
        1,  2,  1,  2,  2,  1,  1,  1,  1,  1,  1,  1,  2,  1,  5
};

void YEEP144_Buzzer_Daisy(void){
    for(int i=0; i<50; ++i){
        float freq = pow(2,(DAISY_P[i]-69)/12.0f)*A440;
        YEEP144_Buzzer_Tone((uint32_t)freq, DAISY_T[i]*QUARTER);
    }
}