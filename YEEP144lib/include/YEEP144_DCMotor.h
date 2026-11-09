#ifndef __YEEP144_DCMOTOR_H
#define __YEEP144_DCMOTOR_H

#include <stdint.h>

// Defines
#define DCMOTOR1 1
#define DCMOTOR2 2
#define DCMOTOR3 3
#define DCMOTOR4 4

#define DCMOTOR_STOP       0 // 0b00
#define DCMOTOR_CCW        1 // 0b01
#define DCMOTOR_CW         2 // 0b10
#define DCMOTOR_SHORTBRAKE 3 // 0b11

#define DCMOTOR_MIN_SPEED 0
#define DCMOTOR_MAX_SPEED 4095 // This corresponds to the Period of the timer

// Function prototypes

// Init functions
void YEEP144_DCMotor_Init(void);
void YEEP144_Encoder1_Init(void);
void YEEP144_Encoder2_Init(void);
void YEEP144_Encoder3_Init(void);
void YEEP144_Encoder4_Init(void);

// Motor functions
void YEEP144_DCMotor_SetSpeed(uint8_t motor, uint16_t speed);
void YEEP144_DCMotor_SetDirection(uint8_t motor, uint8_t direction);

// Encoder functions
uint16_t YEEP144_Encoder1_Get(void);
uint16_t YEEP144_Encoder2_Get(void);
uint16_t YEEP144_Encoder3_Get(void);
uint16_t YEEP144_Encoder4_Get(void);

#endif