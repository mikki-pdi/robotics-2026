#include "YEEP144_PushButton.h"
#include "YEEP144.h"
#include "YEEP144_Pinout.h"
#include "stm32h5xx_hal_cortex.h"
#include "stm32h5xx_hal_def.h"
#include "stm32h5xx_hal_gpio.h"

void YEEP144_PushButton_BlueCallback(void);
void YEEP144_PushButton_GreenCallback(void);
void YEEP144_PushButton_YellowCallback(void);
void YEEP144_PushButton_RedCallback(void);

/**
 * Initialise the blue button. This consumes the button's GPIO and EXTI13
 */
void YEEP144_PushButton_BlueInit(void){
    YEEP144_GPIO_Init(BUTTON_BLUE_PORT, BUTTON_BLUE_PIN, GPIO_MODE_IT_FALLING);
    HAL_NVIC_SetPriority(BUTTON_BLUE_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(BUTTON_BLUE_EXTI);
}

/**
 * Initialise the green button. This consumes the button's GPIO and EXTI7
 */
void YEEP144_PushButton_GreenInit(void){
    YEEP144_GPIO_Init(BUTTON_GREEN_PORT, BUTTON_GREEN_PIN, GPIO_MODE_IT_FALLING);
    HAL_NVIC_SetPriority(BUTTON_GREEN_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(BUTTON_GREEN_EXTI);
}

/**
 * Initialise the yellow button. This consumes the button's GPIO and EXTI6
 */
void YEEP144_PushButton_YellowInit(void){
    YEEP144_GPIO_Init(BUTTON_YELLOW_PORT, BUTTON_YELLOW_PIN, GPIO_MODE_IT_FALLING);
    HAL_NVIC_SetPriority(BUTTON_YELLOW_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(BUTTON_YELLOW_EXTI);
}

/**
 * Initialise the blue button. This consumes the button's GPIO and EXTI12 (shared with ultrasonic 4)
 */
void YEEP144_PushButton_RedInit(void){
    YEEP144_GPIO_Init(BUTTON_RED_PORT, BUTTON_RED_PIN, GPIO_MODE_IT_FALLING);
    HAL_NVIC_SetPriority(BUTTON_RED_EXTI, 1, 0);
    HAL_NVIC_EnableIRQ(BUTTON_RED_EXTI);
}

/**
 * Handles the interrupt for the blue button. This should be called from the main EXTI13 handler, and in turn calls YEEP144_PushButton_BlueCallback 
 */
void YEEP144_PushButton_BlueHandler(void){
    // Check this interrupt has been called due to the pushbutton
    if(__HAL_GPIO_EXTI_GET_IT(BUTTON_BLUE_PIN) && (HAL_GPIO_ReadPin(BUTTON_BLUE_PORT, BUTTON_BLUE_PIN) == GPIO_PIN_RESET)){
        __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_BLUE_PIN); // Clear the interrupt
        YEEP144_PushButton_BlueCallback(); // Call the callback function
    }
}

/**
 * Handles the interrupt for the green button. This should be called from the main EXTI7 handler, and in turn calls YEEP144_PushButton_GreenCallback 
 */
void YEEP144_PushButton_GreenHandler(void){
    // Check this interrupt has been called due to the pushbutton
    if(__HAL_GPIO_EXTI_GET_IT(BUTTON_GREEN_PIN) && (HAL_GPIO_ReadPin(BUTTON_GREEN_PORT, BUTTON_GREEN_PIN) == GPIO_PIN_RESET)){
        __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_GREEN_PIN); // Clear the interrupt
        YEEP144_PushButton_GreenCallback(); // Call the callback function
    }
}

/**
 * Handles the interrupt for the yellow button. This should be called from the main EXTI6 handler, and in turn calls YEEP144_PushButton_YellowCallback 
 */
void YEEP144_PushButton_YellowHandler(void){
    // Check this interrupt has been called due to the pushbutton
    if(__HAL_GPIO_EXTI_GET_IT(BUTTON_YELLOW_PIN) && (HAL_GPIO_ReadPin(BUTTON_YELLOW_PORT, BUTTON_YELLOW_PIN) == GPIO_PIN_RESET)){
        __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_YELLOW_PIN); // Clear the interrupt
        YEEP144_PushButton_YellowCallback(); // Call the callback function
    }
}

/**
 * Handles the interrupt for the red button. This should be called from the main EXTI12 handler, and in turn calls YEEP144_PushButton_RedCallback 
 */
void YEEP144_PushButton_RedHandler(void){
    // Check this interrupt has been called due to the pushbutton
    if(__HAL_GPIO_EXTI_GET_IT(BUTTON_RED_PIN) && (HAL_GPIO_ReadPin(BUTTON_RED_PORT, BUTTON_RED_PIN) == GPIO_PIN_RESET)){
        __HAL_GPIO_EXTI_CLEAR_IT(BUTTON_RED_PIN); // Clear the interrupt
        YEEP144_PushButton_RedCallback(); // Call the callback function
    }
}

/**
 * These versions of the callback functions are weakly defined so they can be overwritten by the user's function elsewhere.
 * They need to exist so that the code will still compile even if the user hasn't defined their own.
 */
__weak void YEEP144_PushButton_BlueCallback(void){

}

__weak void YEEP144_PushButton_GreenCallback(void){

}

__weak void YEEP144_PushButton_YellowCallback(void){

}

__weak void YEEP144_PushButton_RedCallback(void){

}