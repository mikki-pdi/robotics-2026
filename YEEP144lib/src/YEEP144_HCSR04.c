#include "YEEP144_HCSR04.h"
#include "YEEP144.h"
#include "YEEP144_Pinout.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_cortex.h"
#include "stm32h5xx_hal_exti.h"
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_tim.h"

#include <stdint.h>

#define USONIC_PING_LEN 10
#define BLOCKING_TIMEOUT_TICKS 1000

#define USONIC1_MASK (1 << 0)
#define USONIC2_MASK (1 << 1)
#define USONIC3_MASK (1 << 2)
#define USONIC4_MASK (1 << 3)

uint8_t ultrasonicInit = 0;
uint8_t ultrasonicTIMInit = 0;

uint8_t ultrasonicNextPing = USONIC1_MASK;
uint8_t ultrasonicPing = 0;
uint8_t ultrasonicEcho = 0;

uint16_t ultrasonicTickStart[4];
uint16_t ultrasonicTickLen[4];

TIM_HandleTypeDef htim6;

EXTI_HandleTypeDef hexti12;
EXTI_ConfigTypeDef hexti12_conf;

void YEEP144_Ultrasonic_TIMInit(void);

void YEEP144_Ultrasonic1_Callback(uint16_t ticks);
void YEEP144_Ultrasonic2_Callback(uint16_t ticks);
void YEEP144_Ultrasonic3_Callback(uint16_t ticks);
void YEEP144_Ultrasonic4_Callback(uint16_t ticks);


/**
 * Initialises ultrasonic sensor 1. This consumes the two GPIO assigned to that sensor,
 * TIM6 (shared by all 4 ultrasonics), and EXTI line 10 (share by the first 3 ultrasonics)
 */
void YEEP144_Ultrasonic1_Init(void){
    YEEP144_GPIO_Init(USONIC1_TRIG_PORT, USONIC1_TRIG_PIN, GPIO_MODE_OUTPUT);
    HAL_NVIC_SetPriority(USONIC1_ECHO_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(USONIC1_ECHO_EXTI);
    ultrasonicInit |= USONIC1_MASK;

    // Only initialise the timer if it hasn't already
    if(ultrasonicTIMInit == 0){
        YEEP144_Ultrasonic_TIMInit();
    }
}

/**
 * Initialises ultrasonic sensor 2. This consumes the two GPIO assigned to that sensor,
 * TIM6 (shared by all 4 ultrasonics), and EXTI line 10 (share by the first 3 ultrasonics)
 */
void YEEP144_Ultrasonic2_Init(void){
    YEEP144_GPIO_Init(USONIC2_TRIG_PORT, USONIC2_TRIG_PIN, GPIO_MODE_OUTPUT);
    HAL_NVIC_SetPriority(USONIC2_ECHO_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(USONIC2_ECHO_EXTI);
    ultrasonicInit |= USONIC2_MASK;

    // Only initialise the timer if it hasn't already
    if(ultrasonicTIMInit == 0){
        YEEP144_Ultrasonic_TIMInit();
    }
}

/**
 * Initialises ultrasonic sensor 3. This consumes the two GPIO assigned to that sensor,
 * TIM6 (shared by all 4 ultrasonics), and EXTI line 10 (share by the first 3 ultrasonics)
 */
void YEEP144_Ultrasonic3_Init(void){
    YEEP144_GPIO_Init(USONIC3_TRIG_PORT, USONIC3_TRIG_PIN, GPIO_MODE_OUTPUT);
    HAL_NVIC_SetPriority(USONIC3_ECHO_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(USONIC4_ECHO_EXTI);
    ultrasonicInit |= USONIC3_MASK;

    // Only initialise the timer if it hasn't already
    if(ultrasonicTIMInit == 0){
        YEEP144_Ultrasonic_TIMInit();
    }
}

/**
 * Initialises ultrasonic sensor 4. This consumes the two GPIO assigned to that sensor,
 * TIM6 (shared by all 4 ultrasonics), and EXTI line 12 (shared with button 3 / red)
 */
void YEEP144_Ultrasonic4_Init(void){
    YEEP144_GPIO_Init(USONIC4_TRIG_PORT, USONIC4_TRIG_PIN, GPIO_MODE_OUTPUT);
    HAL_NVIC_SetPriority(USONIC4_ECHO_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(USONIC4_ECHO_EXTI);
    ultrasonicInit |= USONIC4_MASK;

    // Only initialise the timer if it hasn't already
    if(ultrasonicTIMInit == 0){
        YEEP144_Ultrasonic_TIMInit();
    }
}

/**
 * Starts a reading on the next sensor in the sequence using the non-blocking method, skipping over any uninitialised sensors
 * Only 1 sensor can be read at a time as they (mostly) share the same EXTI line, so this function
 * checks to make sure that noting else is currently pinging.
 *
 * If no sensor is currently initialised returns 255
 * If a sensor is already pinging, return 0
 * If a new reading is successfully started, return the status byte indicating which sensor is currently pinging
 *
 * The relevant callback function will be called when pinging finishes and a new distance value is available
 */
uint8_t YEEP144_Ultrasonic_PingNext(void){

    // If nothing is currently pinging
    if(!ultrasonicPing){
        // What is the next in the sequence
        switch(ultrasonicNextPing){
            case USONIC1_MASK:
                // Only start if it is enabled
                if(ultrasonicInit&USONIC1_MASK){
                    // Reinitialise the pin to setup the interrupt
                    YEEP144_GPIO_Init(USONIC1_ECHO_PORT, USONIC1_ECHO_PIN, GPIO_MODE_IT_RISING_FALLING);

                    // Toggle the pin on for 10uS
                    HAL_GPIO_WritePin(USONIC1_TRIG_PORT, USONIC1_TRIG_PIN, GPIO_PIN_SET);
                    delayUS(USONIC_PING_LEN);
                    HAL_GPIO_WritePin(USONIC1_TRIG_PORT, USONIC1_TRIG_PIN, GPIO_PIN_RESET);

                    // Set the ping bit so we know this sensor is pinging
                    ultrasonicPing |= USONIC1_MASK;

                    // Move to next sensor
                    ultrasonicNextPing = USONIC2_MASK;
                    break; // Break is within the if statement, so if a sensor is not initialised it will fall through to the next
                }
                else{
                    // Move to next sensor
                    ultrasonicNextPing = USONIC2_MASK;
                }
            // fallthrough
            case USONIC2_MASK:
                if(ultrasonicInit&USONIC2_MASK){
                    // Reinitialise the pin to setup the interrupt
                    YEEP144_GPIO_Init(USONIC2_ECHO_PORT, USONIC2_ECHO_PIN, GPIO_MODE_IT_RISING_FALLING);

                    // Toggle the pin on for 10uS
                    HAL_GPIO_WritePin(USONIC2_TRIG_PORT, USONIC2_TRIG_PIN, GPIO_PIN_SET);
                    delayUS(USONIC_PING_LEN);
                    HAL_GPIO_WritePin(USONIC2_TRIG_PORT, USONIC2_TRIG_PIN, GPIO_PIN_RESET);

                    // Set the ping bit so we know this sensor is pinging
                    ultrasonicPing |= USONIC2_MASK;

                    // Move to next sensor
                    ultrasonicNextPing = USONIC3_MASK;
                    break; // Break is within the if statement, so if a sensor is not initialised it will fall through to the next
                }
                else{
                    // Move to next sensor
                    ultrasonicNextPing = USONIC3_MASK;
                }
            // fallthrough
            case USONIC3_MASK:
                if(ultrasonicInit&USONIC3_MASK){
                    // Reinitialise the pin to setup the interrupt
                    YEEP144_GPIO_Init(USONIC3_ECHO_PORT, USONIC3_ECHO_PIN, GPIO_MODE_IT_RISING_FALLING);

                    // Toggle the pin on for 10uS
                    HAL_GPIO_WritePin(USONIC3_TRIG_PORT, USONIC3_TRIG_PIN, GPIO_PIN_SET);
                    delayUS(USONIC_PING_LEN);
                    HAL_GPIO_WritePin(USONIC3_TRIG_PORT, USONIC3_TRIG_PIN, GPIO_PIN_RESET);

                    // Set the ping bit so we know this sensor is pinging
                    ultrasonicPing |= USONIC3_MASK;

                    // Move to next sensor
                    ultrasonicNextPing = USONIC4_MASK;
                    break; // Break is within the if statement, so if a sensor is not initialised it will fall through to the next
                }
                else{
                    // Move to next sensor
                    ultrasonicNextPing = USONIC4_MASK;
                }
            // fallthrough
            case USONIC4_MASK:
                if(ultrasonicInit&USONIC4_MASK){
                    // As the interrupt line for ultrasonic 4 is shared with pushbutton 3, 
                    // we want to save the EXTI configuration to restore when pinging is finished
                    HAL_EXTI_GetHandle(&hexti12, EXTI_LINE_12);
                    HAL_EXTI_GetConfigLine(&hexti12, &hexti12_conf);

                    // Reinitialise the pin to setup the interrupt
                    YEEP144_GPIO_Init(USONIC4_ECHO_PORT, USONIC4_ECHO_PIN, GPIO_MODE_IT_RISING_FALLING);

                    // Toggle the pin on for 10uS
                    HAL_GPIO_WritePin(USONIC4_TRIG_PORT, USONIC4_TRIG_PIN, GPIO_PIN_SET);
                    delayUS(USONIC_PING_LEN);
                    HAL_GPIO_WritePin(USONIC4_TRIG_PORT, USONIC4_TRIG_PIN, GPIO_PIN_RESET);

                    // Set the ping bit so we know this sensor is pinging
                    ultrasonicPing |= USONIC4_MASK;

                    // Move to next sensor
                    ultrasonicNextPing = USONIC1_MASK;
                    break; // Break is within the if statement, so if a sensor is not initialised it will fall through to the next
                }
                else{
                    // Move to next sensor
                    ultrasonicNextPing = USONIC1_MASK;
                }

        }
        // If a ping was successfully started return the status byte, else return 255
        if(!ultrasonicPing){
            return ultrasonicPing;
        }
        else{
            // This should only happen if this function is called with no sensor initialised
            return 255;
        }
    }
    else{
        // Returns 0 if a sensor is already pinging
        return 0;
    }
}

/**
 * Starts a reading on ultrasonic 1 using the non-blocking method
 *
 * If no sensor is currently initialised returns 255
 * If a sensor is already pinging, return 0
 * If a new reading is successfully started, return the status byte indicating which sensor is currently pinging
 *
 * YEEP144_Ultrasonic1_Callback will be called when a new value is available
 */
uint8_t YEEP144_Ultrasonic1_Ping(void){
    ultrasonicNextPing = USONIC1_MASK;
    return YEEP144_Ultrasonic_PingNext();
}

/**
 * Starts a reading on ultrasonic 2 using the non-blocking method
 *
 * If no sensor is currently initialised returns 255
 * If a sensor is already pinging, return 0
 * If a new reading is successfully started, return the status byte indicating which sensor is currently pinging
 *
 * YEEP144_Ultrasonic2_Callback will be called when a new value is available
 */
uint8_t YEEP144_Ultrasonic2_Ping(void){
    ultrasonicNextPing = USONIC2_MASK;
    return YEEP144_Ultrasonic_PingNext();
}

/**
 * Starts a reading on ultrasonic 3 using the non-blocking method
 *
 * If no sensor is currently initialised returns 255
 * If a sensor is already pinging, return 0
 * If a new reading is successfully started, return the status byte indicating which sensor is currently pinging
 *
 * YEEP144_Ultrasonic3_Callback will be called when a new value is available
 */
uint8_t YEEP144_Ultrasonic3_Ping(void){
    ultrasonicNextPing = USONIC3_MASK;
    return YEEP144_Ultrasonic_PingNext();
}

/**
 * Starts a reading on ultrasonic 4 using the non-blocking method
 *
 * If no sensor is currently initialised returns 255
 * If a sensor is already pinging, return 0
 * If a new reading is successfully started, return the status byte indicating which sensor is currently pinging
 *
 * YEEP144_Ultrasonic4_Callback will be called when a new value is available
 */
uint8_t YEEP144_Ultrasonic4_Ping(void){
    ultrasonicNextPing = USONIC4_MASK;
    return YEEP144_Ultrasonic_PingNext();
}

/**
 * Starts a reading on ultrasonic 1 with blocking waits until the new value is available
 *
 * Returns 0 if the sensor is not initialised, or the function times out, otherwise returns the tick value
 *
 * YEEP144_Ultrasonic1_Callback will still be called when the new value is available
 *
 */
uint16_t YEEP144_Ultrasonic1_PingBlocking(void){

    uint32_t blockingTimeoutStart = HAL_GetTick();

    uint8_t pingReturn;
    // Attempt pings until successful
    // If 255 is returned, sensor is uninitialised, so fail and return 0
    // If we exceed timeout then return 0;
    do{
        pingReturn = YEEP144_Ultrasonic1_Ping();
        if(pingReturn == 255) return 0;
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    } while(pingReturn != USONIC1_MASK);

    // Wait until pinging starts
    // If we exceed timeout then return 0
    while(ultrasonicPing != USONIC1_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Wait until pinging ends
    // If we exceed timeout then return 0
    while(ultrasonicPing == USONIC1_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Return the read value
    return ultrasonicTickLen[0];
}

/**
 * Starts a reading on ultrasonic 2 with blocking waits until the new value is available
 *
 * Returns 0 if the sensor is not initialised, or the function times out, otherwise returns the tick value
 *
 * YEEP144_Ultrasonic1_Callback will still be called when the new value is available
 *
 */
uint16_t YEEP144_Ultrasonic2_PingBlocking(void){

    uint32_t blockingTimeoutStart = HAL_GetTick();

    uint8_t pingReturn;
    // Attempt pings until successful
    // If 255 is returned, sensor is uninitialised, so fail and return 0
    // If we exceed timeout then return 0;
    do{
        pingReturn = YEEP144_Ultrasonic2_Ping();
        if(pingReturn == 255) return 0;
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    } while(pingReturn != USONIC2_MASK);

    // Wait until pinging starts
    // If we exceed timeout then return 0
    while(ultrasonicPing != USONIC2_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Wait until pinging ends
    // If we exceed timeout then return 0
    while(ultrasonicPing == USONIC2_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Return the read value
    return ultrasonicTickLen[1];
}

/**
 * Starts a reading on ultrasonic 3 with blocking waits until the new value is available
 *
 * Returns 0 if the sensor is not initialised, or the function times out, otherwise returns the tick value
 *
 * YEEP144_Ultrasonic1_Callback will still be called when the new value is available
 *
 */
uint16_t YEEP144_Ultrasonic3_PingBlocking(void){

    uint32_t blockingTimeoutStart = HAL_GetTick();

    uint8_t pingReturn;
    // Attempt pings until successful
    // If 255 is returned, sensor is uninitialised, so fail and return 0
    // If we exceed timeout then return 0;
    do{
        pingReturn = YEEP144_Ultrasonic3_Ping();
        if(pingReturn == 255) return 0;
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    } while(pingReturn != USONIC3_MASK);

    // Wait until pinging starts
    // If we exceed timeout then return 0
    while(ultrasonicPing != USONIC3_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Wait until pinging ends
    // If we exceed timeout then return 0
    while(ultrasonicPing == USONIC3_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Return the read value
    return ultrasonicTickLen[2];
}

/**
 * Starts a reading on ultrasonic 4 with blocking waits until the new value is available
 *
 * Returns 0 if the sensor is not initialised, or the function times out, otherwise returns the tick value
 *
 * YEEP144_Ultrasonic1_Callback will still be called when the new value is available
 *
 */
uint16_t YEEP144_Ultrasonic4_PingBlocking(void){

    uint32_t blockingTimeoutStart = HAL_GetTick();

    uint8_t pingReturn;
    // Attempt pings until successful
    // If 255 is returned, sensor is uninitialised, so fail and return 0
    // If we exceed timeout then return 0;
    do{
        pingReturn = YEEP144_Ultrasonic4_Ping();
        if(pingReturn == 255) return 0;
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    } while(pingReturn != USONIC4_MASK);

    // Wait until pinging starts
    // If we exceed timeout then return 0
    while(ultrasonicPing != USONIC4_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Wait until pinging ends
    // If we exceed timeout then return 0
    while(ultrasonicPing == USONIC4_MASK){
        if((HAL_GetTick()- blockingTimeoutStart) >= BLOCKING_TIMEOUT_TICKS) return 0;
    }

    // Return the read value
    return ultrasonicTickLen[3];
}

/**
* This function handles the interrupt for sensors 1, 2, and 3 connected to EXTI 10
* This should be called from the main EXTI10 IRQ handler found in YEEP144_IRQ.c
*/
void YEEP144_Ultrasonic_EXTI10Handler(void){
    // Store the value from the timer at the start of the ISR to avoid execution time affecting the result
    uint16_t TIMVal = __HAL_TIM_GET_COUNTER(&htim6);

    // Rising edge interrupt means pulse is starting
    if(__HAL_GPIO_EXTI_GET_RISING_IT(USONIC1_ECHO_PIN)){
        // Check to see which sensor is currently pinging based on the GPIO and the ping status byte
        if((HAL_GPIO_ReadPin(USONIC1_ECHO_PORT, USONIC1_ECHO_PIN) == GPIO_PIN_SET) && (ultrasonicPing & USONIC1_MASK)){
            // Store the starting timer value and set the relevant status bit
            ultrasonicTickStart[0] = TIMVal;
            ultrasonicEcho |= USONIC1_MASK;
        }
        if((HAL_GPIO_ReadPin(USONIC2_ECHO_PORT, USONIC2_ECHO_PIN) == GPIO_PIN_SET) && (ultrasonicPing & USONIC2_MASK)){
            ultrasonicTickStart[1] = TIMVal;
            ultrasonicEcho |= USONIC2_MASK;
        }
        if((HAL_GPIO_ReadPin(USONIC3_ECHO_PORT, USONIC3_ECHO_PIN) == GPIO_PIN_SET) && (ultrasonicPing & USONIC3_MASK)){
            ultrasonicTickStart[2] = TIMVal;
            ultrasonicEcho |= USONIC3_MASK;
        }
        __HAL_GPIO_EXTI_CLEAR_RISING_IT(USONIC1_ECHO_PIN); // Clear the interrupt
        return;
    }
    // Falling edge interrupt means pulse is ending
    else if(__HAL_GPIO_EXTI_GET_FALLING_IT(USONIC1_ECHO_PIN)){
        // Check which sensor is pinging based on the GPIO and status byte
        if((HAL_GPIO_ReadPin(USONIC1_ECHO_PORT, USONIC1_ECHO_PIN) == GPIO_PIN_RESET) && (ultrasonicEcho & USONIC1_MASK)){
            // Reset the status bits
            ultrasonicPing &= ~USONIC1_MASK;
            ultrasonicEcho &= ~USONIC1_MASK;
            ultrasonicTickLen[0] = TIMVal - ultrasonicTickStart[0]; // Calculate the time elapsed
            YEEP144_Ultrasonic1_Callback(ultrasonicTickLen[0]); // Call the relevant callback function to signal the reading is done
        }
        if((HAL_GPIO_ReadPin(USONIC2_ECHO_PORT, USONIC2_ECHO_PIN) == GPIO_PIN_RESET) && (ultrasonicEcho & USONIC2_MASK)){
            ultrasonicPing &= ~USONIC2_MASK;
            ultrasonicEcho &= ~USONIC2_MASK;
            ultrasonicTickLen[1] = TIMVal - ultrasonicTickStart[1];
            YEEP144_Ultrasonic2_Callback(ultrasonicTickLen[1]);
        }
        if((HAL_GPIO_ReadPin(USONIC3_ECHO_PORT, USONIC3_ECHO_PIN) == GPIO_PIN_RESET) && (ultrasonicEcho & USONIC3_MASK)){
            ultrasonicPing &= ~USONIC3_MASK;
            ultrasonicEcho &= ~USONIC3_MASK;
            ultrasonicTickLen[2] = TIMVal - ultrasonicTickStart[2];
            YEEP144_Ultrasonic3_Callback(ultrasonicTickLen[2]);
        }
        __HAL_GPIO_EXTI_CLEAR_FALLING_IT(USONIC1_ECHO_PIN); // Clear the interrupt
        return;
    }
}

/**
* This function handles the interrupt for sensor 4 connected to EXTI 12
* This should be called from the main EXTI12 IRQ handler found in YEEP144_IRQ.c
*/
void YEEP144_Ultrasonic_EXTI12Handler(void){
    // Store the value from the timer at the start of the ISR to avoid execution time affecting the result
    uint16_t TIMVal = __HAL_TIM_GET_COUNTER(&htim6);

    // Rising edge interrupt means pulse is starting
    if(__HAL_GPIO_EXTI_GET_RISING_IT(USONIC4_ECHO_PIN)){
        // Check to see which sensor is currently pinging based on the GPIO and the ping status byte
        if((HAL_GPIO_ReadPin(USONIC4_ECHO_PORT, USONIC4_ECHO_PIN) == GPIO_PIN_SET) && (ultrasonicPing & USONIC4_MASK)){
            // Store the starting timer value and set the relevant status bit
            ultrasonicTickStart[3] = TIMVal;
            ultrasonicEcho |= USONIC4_MASK;
        }
        __HAL_GPIO_EXTI_CLEAR_RISING_IT(USONIC4_ECHO_PIN); // Clear the interrupt
        return;
    }
    // Falling edge interrupt means pulse is ending
    else if(__HAL_GPIO_EXTI_GET_FALLING_IT(USONIC4_ECHO_PIN)){
        // Check which sensor is pinging based on the GPIO and status byte
        if((HAL_GPIO_ReadPin(USONIC4_ECHO_PORT, USONIC4_ECHO_PIN) == GPIO_PIN_RESET) && (ultrasonicEcho & USONIC4_MASK)){
            // Reset the status bits
            ultrasonicPing &= ~USONIC4_MASK;
            ultrasonicEcho &= ~USONIC4_MASK;
            ultrasonicTickLen[3] = TIMVal - ultrasonicTickStart[3]; // Calculate the time elapsed
            YEEP144_Ultrasonic4_Callback(ultrasonicTickLen[3]); // Call the relevant callback function to signal the reading is done
        }
        __HAL_GPIO_EXTI_CLEAR_FALLING_IT(USONIC4_ECHO_PIN); // Clear the interrupt
        // Restore the previous exti configuration
        HAL_EXTI_GetHandle(&hexti12, EXTI_LINE_12);
        HAL_EXTI_SetConfigLine(&hexti12, &hexti12_conf);
        return;
    }
}

/**
 * Helper function to convert timer ticks to a centimeter value
 */
float YEEP144_Ultrasonic_TickTocm(uint16_t ticks){
    // Total number of ticks is return trip time, we halve it to get distance to object
    // Speed of sound taken as 343m/s = 34300 cm/s = 0.0343 cm/us
    return ticks/2.0 * 0.0343;
}

/**
* Internal function used to initialise the timer used for measuring the received pulses
*/
void YEEP144_Ultrasonic_TIMInit(void){

    // Enable the clock to the timer
    __HAL_RCC_TIM6_CLK_ENABLE();

    // Create empty config structure
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // Populate config structure. The should configure it for 1uS per tick, assuming 250MHz peripheral clock
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 249;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 65535;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK){
        //Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK){
        //Error_Handler();
    }

    // Start the counting. This counter is left free running as relevant functions 
    // calculate time elapsed by recording the value at the start and end of an event
    HAL_TIM_Base_Start(&htim6);

    // Set the init flag so the timer doesn't get initialised multiple times
    ultrasonicTIMInit = 1;
}


/**
 * These callback functions are weakly defined so they can be overwritten by the user's
 * own callback function. They have to exist at least in weak form so that the code compiles
 * even if the user chooses not to define their own
 */
__weak void YEEP144_Ultrasonic1_Callback(uint16_t ticks){

}

__weak void YEEP144_Ultrasonic2_Callback(uint16_t ticks){

}

__weak void YEEP144_Ultrasonic3_Callback(uint16_t ticks){

}

__weak void YEEP144_Ultrasonic4_Callback(uint16_t ticks){

}