#include "YEEP144.h"
#include "YEEP144_Servo.h"

#include "YEEP144_Pinout.h"
#include "stm32h563xx.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_rcc.h"
#include "stm32h5xx_hal_tim.h"

#include <stdio.h>

TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim17servo;

uint16_t servoMinTicks[4] = {2048, 2048, 2048, 2048};
uint16_t servoMaxTicks[4] = {4095, 4095, 4095, 4095};
float servoMinDegrees[4] = {0, 0, 0, 0};
float servoMaxDegrees[4] = {180, 180, 180, 180};

uint16_t YEEP144_Servo_MapAngleToTicks(float angle, float minAngle, float maxAngle, uint16_t minTicks, uint16_t maxTicks);

/**
 * Initialises servos 1 and 2. This consumes timer 5 and two GPIO
 */
void YEEP144_Servo_Init1_2(void){

    // Enable clock to TIM5
    __HAL_RCC_TIM5_CLK_ENABLE();

    // Configure and initialise TIM5
    // Servo PWM is a 1ms - 2ms pulse every 20ms
    // This is done using  a 50Hz PWM with a duty cycle between 5 and 10%
    // Our ARR is set to 40950, the range of the ADC is 0-4095, /2 = 2048, which represents 5% of the total range
    // Offsetting by 2048 means we cover the 5 to 10% range
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 121;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 40950;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim5) != HAL_OK){
    //Error_Handler();
    }

    // Disable synchronisation as it is not needed
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK){
    //Error_Handler();
    }

    // Configure the two used PWM channels
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    //Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4) != HAL_OK){
    //Error_Handler();
    }

    // Configure GPIOs
    YEEP144_GPIO_InitAdv(SERVO1_PORT, SERVO1_PIN | SERVO2_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF2_TIM5);

    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);

}

/**
 * Initialises servo 3. This consumes timer 13 and one GPIO
 */
void YEEP144_Servo_Init3(void){
    // Enable clock to TIM13
    __HAL_RCC_TIM13_CLK_ENABLE();

    // Configure and initialise TIM13
    // Servo PWM is a 1ms - 2ms pulse every 20ms
    // This is done using  a 50Hz PWM with a duty cycle between 5 and 10%
    // Our ARR is set to 40950, the range of the ADC is 0-4095, /2 = 2048, which represents 5% of the total range
    // Offsetting by 2048 means we cover the 5 to 10% range
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim13.Instance = TIM13;
    htim13.Init.Prescaler = 121;
    htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim13.Init.Period = 40950;
    htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim13) != HAL_OK){
    //Error_Handler();
    }

    // Disable synchronisation as it is not needed
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim13, &sMasterConfig) != HAL_OK){
    //Error_Handler();
    }

    // Configure the two used PWM channels
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim13, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    //Error_Handler();
    }

    // Configure GPIOs
    YEEP144_GPIO_InitAdv(SERVO3_PORT, SERVO3_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF9_TIM13);

    HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);
}

/**
 * Initialises servo 4. This consumes timer 17 and one GPIO. These resources are share with the buzzer.
 * Ensure only one is initialised. When using servo 4, disable the buzzer using DIP switch SW5 so the
 * PWM signal is not made audible
 */
void YEEP144_Servo_Init4(void){
    // Enable clock to TIM17
    __HAL_RCC_TIM17_CLK_ENABLE();

    // Configure and initialise TIM17
    // Servo PWM is a 1ms - 2ms pulse every 20ms
    // This is done using  a 50Hz PWM with a duty cycle between 5 and 10%
    // Our ARR is set to 40950, the range of the ADC is 0-4095, /2 = 2048, which represents 5% of the total range
    // Offsetting by 2048 means we cover the 5 to 10% range
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim17servo.Instance = TIM17;
    htim17servo.Init.Prescaler = 121;
    htim17servo.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim17servo.Init.Period = 40950;
    htim17servo.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim17servo.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim17servo) != HAL_OK){
    //Error_Handler();
    }

    // Disable synchronisation as it is not needed
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim17servo, &sMasterConfig) != HAL_OK){
    //Error_Handler();
    }

    // Configure the two used PWM channels
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim17servo, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    //Error_Handler();
    }

    // Configure GPIOs
    YEEP144_GPIO_InitAdv(SERVO4_PORT, SERVO4_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, GPIO_AF1_TIM17);

    HAL_TIM_PWM_Start(&htim17servo, TIM_CHANNEL_1);
}

/** 
 * Sets the zero point for a given servo
 *
 * Default configuration is 2048, yielding a 5% to 10% duty cycle, or 1ms - 2ms pulse, which is the typical standard
 * Adjusting the zero offset can help account for differences in servos
 * that means 5% duty cycle does not exactly reach 0 degrees
 *
 * The offset applies to both SetDirect and SetAngle
 */
void YEEP144_Servo_SetMinTicks(uint8_t servo, uint16_t min){
    if(servo == 0 || servo >= 4) return;
    servoMinTicks[servo-1] = min;
}

/** 
 * Sets the max point for a given servo
 *
 * Default configuration is 4096, yielding a 5% to 10% duty cycle, or 1ms - 2ms pulse, which is the typical standard
 * Adjusting the max offset can help account for differences in servos
 * that means 10% duty cycle does not exactly reach 180 degrees
 *
 * This only applies when using SetAngle. SetDirect has no limit on what you can set
 */
void YEEP144_Servo_SetMaxTicks(uint8_t servo, uint16_t max){
    if(servo == 0 || servo >= 4) return;
    servoMaxTicks[servo-1] = max;
}

/**
 * Sets the minimum angle range for a given servo
 *
 * Default is 0, for 0-180deg servos
 *
 * This only applies to SetAngle
 */
void YEEP144_Servo_SetMinAngle(uint8_t servo, float min){
    if(servo == 0 || servo >= 4) return;
    servoMinDegrees[servo-1] = min;
}

/**
 * Sets the maximum angle range for a given servo
 *
 * Default is 180, for 0-180deg servos
 *
 * This only applies to SetAngle
 */
void YEEP144_Servo_SetMaxAngle(uint8_t servo, float max){
    if(servo == 0 || servo >= 4) return;
    servoMaxDegrees[servo-1] = max;
}

/**
 * Directly set the pulse value of the timer to control the position of the servo
 * Typically this should be in the range 0-2047
 * The function will accept larger pulse values, which will result in a duty cycle >10%, or a pulse of >2ms
 * This may be useful for servos who's maximum angle is not achieved at 10%
 * Along with setting the zero offset, this should account for troublesome servos, albeit with less elegant maths
 */
void YEEP144_Servo1_SetDirect(uint16_t pulse){
    TIM5->CCR1 = pulse + servoMinTicks[0];
}

/**
 * Directly set the pulse value of the timer to control the position of the servo
 * Typically this should be in the range 0-2047
 * The function will accept larger pulse values, which will result in a duty cycle >10%, or a pulse of >2ms
 * This may be useful for servos who's maximum angle is not achieved at 10%
 * Along with setting the zero offset, this should account for troublesome servos, albeit with less elegant maths
 */
void YEEP144_Servo2_SetDirect(uint16_t pulse){
    TIM5->CCR4 = pulse + servoMinTicks[1];
}

/**
 * Directly set the pulse value of the timer to control the position of the servo
 * Typically this should be in the range 0-2047
 * The function will accept larger pulse values, which will result in a duty cycle >10%, or a pulse of >2ms
 * This may be useful for servos who's maximum angle is not achieved at 10%
 * Along with setting the zero offset, this should account for troublesome servos, albeit with less elegant maths
 */
void YEEP144_Servo3_SetDirect(uint16_t pulse){
    TIM13->CCR1 = pulse + servoMinTicks[2];
}

/**
 * Directly set the pulse value of the timer to control the position of the servo
 * Typically this should be in the range 0-2047
 * The function will accept larger pulse values, which will result in a duty cycle >10%, or a pulse of >2ms
 * This may be useful for servos who's maximum angle is not achieved at 10%
 * Along with setting the zero offset, this should account for troublesome servos, albeit with less elegant maths
 */
void YEEP144_Servo4_SetDirect(uint16_t pulse){
    TIM17->CCR1 = pulse + servoMinTicks[3];
}

/**
 * Set the servo to a specified angle. This will be based on the minimum and maximum angles set for the servo
 * By default this will work in the range 0 to 180
 */
void YEEP144_Servo1_SetAngle(float angle){
    if(angle < servoMinDegrees[0] || angle > servoMaxDegrees[0]) return; // If the angle is out of range, do nothing
    TIM5->CCR1 = YEEP144_Servo_MapAngleToTicks(angle, servoMinDegrees[0], servoMaxDegrees[0], servoMinTicks[0], servoMaxTicks[0]);
}

/**
 * Set the servo to a specified angle. This will be based on the minimum and maximum angles set for the servo
 * By default this will work in the range 0 to 180
 */
void YEEP144_Servo2_SetAngle(float angle){
    if(angle < servoMinDegrees[1] || angle > servoMaxDegrees[1]) return; // If the angle is out of range, do nothing
    TIM5->CCR1 = YEEP144_Servo_MapAngleToTicks(angle, servoMinDegrees[1], servoMaxDegrees[1], servoMinTicks[1], servoMaxTicks[1]);
}

/**
 * Set the servo to a specified angle. This will be based on the minimum and maximum angles set for the servo
 * By default this will work in the range 0 to 180
 */
void YEEP144_Servo3_SetAngle(float angle){
    if(angle < servoMinDegrees[0] || angle > servoMaxDegrees[0]) return; // If the angle is out of range, do nothing
    TIM5->CCR1 = YEEP144_Servo_MapAngleToTicks(angle, servoMinDegrees[0], servoMaxDegrees[0], servoMinTicks[0], servoMaxTicks[0]);
}

/**
 * Set the servo to a specified angle. This will be based on the minimum and maximum angles set for the servo
 * By default this will work in the range 0 to 180
 */
void YEEP144_Servo4_SetAngle(float angle){
    if(angle < servoMinDegrees[0] || angle > servoMaxDegrees[0]) return; // If the angle is out of range, do nothing
    TIM5->CCR1 = YEEP144_Servo_MapAngleToTicks(angle, servoMinDegrees[0], servoMaxDegrees[0], servoMinTicks[0], servoMaxTicks[0]);
}

/**
 * Helper function used to map a float angle to a timer tick value, based on the min and max ranges specified
 * e.g. map the angle 90 from the range 0 to 180, into the range 2048 - 4095
 */
uint16_t YEEP144_Servo_MapAngleToTicks(float angle, float minAngle, float maxAngle, uint16_t minTicks, uint16_t maxTicks){
    // Map the angle value into the tick range using the min and max parameters
    return (uint16_t)(( ((angle - minAngle) * (maxTicks - minTicks)) / (maxAngle - minAngle) ) + minTicks);
}