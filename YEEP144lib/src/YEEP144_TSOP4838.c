#include "YEEP144.h"
#include "YEEP144_Pinout.h"
#include "YEEP144_DebugUART.h"

#define ASCII_ESC 27

// NEC Protocol Timing Constants (in microseconds)
#define NEC_START_LOW     9000   // 9ms leading pulse
#define NEC_START_HIGH    4500   // 4.5ms space
#define NEC_BIT_LOW       560    // 560us burst
#define NEC_BIT_ONE_HIGH  1690   // 1.69ms space for logical 1
#define NEC_BIT_ZERO_HIGH 560    // 560us space for logical 0
#define NEC_TOLERANCE     400    // +/- margin of error

// State Machine States
typedef enum{
    IR_STATE_IDLE,
    IR_STATE_LEADER,
    IR_STATE_DATA
} IR_State_t;

// Variables
volatile IR_State_t ir_state = IR_STATE_IDLE;
volatile uint32_t ir_last_time = 0;
volatile uint32_t ir_raw_bits = 0;
volatile uint8_t ir_bit_count = 0;
volatile uint8_t ir_code_ready = 0;
volatile uint32_t ir_code = 0;

// Enable hardware DWT (Data Watchpoint and Trace) cycle counter 
void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// Arduino-style microsecond measurement
uint32_t micros(void) {
    return DWT->CYCCNT / (SystemCoreClock / 1000000);
}

// This exact function name matches the STM32H563 hardware vector table entry
void EXTI3_IRQHandler(void)
{
    // Tells the HAL layer to clear the flag and fire your callback routine
    HAL_GPIO_EXTI_IRQHandler(IR_RECEIVER_PIN);
}

void YEEP144_TSOP4838_Init(){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable DWT for microsecond timing
    DWT_Init();

    // Configure Pin F3 for Interrupt on Both Edges
    GPIO_InitStruct.Pin = IR_RECEIVER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // Triggers on rising or falling change
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // Uses the TSOP4838 internal pull-up
    HAL_GPIO_Init(IR_RECEIVER_PORT, &GPIO_InitStruct);

    // Enable and set EXTI Line 3 Vector for STM32H563
    HAL_NVIC_SetPriority(IR_RECEIVER_EXTI, 2, 0); 
    HAL_NVIC_EnableIRQ(IR_RECEIVER_EXTI);
}

uint8_t YEEP144_TSOP4838_Received(void){
    return ir_code_ready;
}

uint32_t YEEP144_TSOP4838_Read(void){
    return ir_code;
    ir_code_ready = 0;
}

// These functions override the weak HAL geometric EXTI callbacks
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
    if (GPIO_Pin == IR_RECEIVER_PIN)
    {
        uint32_t current_time = micros();
        uint32_t duration = current_time - ir_last_time;
        ir_last_time = current_time;
        switch(ir_state) {
            case IR_STATE_LEADER: // Rising edge (End of 9ms pulse)
                if (duration < (NEC_START_LOW - NEC_TOLERANCE) || duration > (NEC_START_LOW + NEC_TOLERANCE)){
                    ir_state = IR_STATE_IDLE; // False start
                }
                break;
            case IR_STATE_IDLE:
                break;
            case IR_STATE_DATA:
                break;
        }
    }
}

// For newer STM32 HAL libraries, rising and falling callbacks are separated for memory efficiency 
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin){
    if (GPIO_Pin == IR_RECEIVER_PIN)
    {
        uint32_t current_time = micros();
        uint32_t duration = current_time - ir_last_time;
        ir_last_time = current_time;
        switch(ir_state){
            case IR_STATE_IDLE: // Falling edge (Start of 9ms pulse)
                ir_state = IR_STATE_LEADER;
                break;
            case IR_STATE_LEADER: // Falling edge (End of 4.5ms space)
                if (duration >= (NEC_START_HIGH - NEC_TOLERANCE) && duration <= (NEC_START_HIGH + NEC_TOLERANCE)){
                    ir_state = IR_STATE_DATA;
                    ir_bit_count = 0;
                    ir_raw_bits = 0;
                } else {
                    ir_state = IR_STATE_IDLE; // Invalid preamble
                }
                break;
            case IR_STATE_DATA: // Falling edge (End of a data bit's high space)
                // The high duration determines if the bit is a 0 or a 1
                if (duration >= (NEC_BIT_ZERO_HIGH - NEC_TOLERANCE) && duration <= (NEC_BIT_ZERO_HIGH + NEC_TOLERANCE)) {
                    ir_raw_bits >>= 1; // It's a 0
                    ir_bit_count++;
                } 
                else if (duration >= (NEC_BIT_ONE_HIGH - NEC_TOLERANCE) && duration <= (NEC_BIT_ONE_HIGH + NEC_TOLERANCE)) {
                    ir_raw_bits >>= 1;
                    ir_raw_bits |= 0x80000000; // It's a 1
                    ir_bit_count++;
                } 
                else {
                    ir_state = IR_STATE_IDLE; // Timing error, reset state
                }

                uint32_t ir_bits = 0;
                if (ir_bit_count == 32) {
                    ir_bits = ir_raw_bits;
                    ir_code = (ir_bits >> 16) & 0xFF;
                    uint32_t command_inverse = (ir_bits >> 24) & 0xFF;
                    if (ir_code == (uint8_t)(~command_inverse))
                        ir_code_ready = 1;
                    ir_state = IR_STATE_IDLE;
                }
            break;
        }
    }
}
