#ifndef __YEEP144_HCSR04_H
#define __YEEP144_HCSR04_H

#include <stdint.h>

void YEEP144_Ultrasonic1_Init(void);
void YEEP144_Ultrasonic2_Init(void);
void YEEP144_Ultrasonic3_Init(void);
void YEEP144_Ultrasonic4_Init(void);

uint8_t YEEP144_Ultrasonic_PingNext(void);
float YEEP144_Ultrasonic_TickTocm(uint16_t ticks);

uint8_t YEEP144_Ultrasonic1_Ping(void);
uint8_t YEEP144_Ultrasonic2_Ping(void);
uint8_t YEEP144_Ultrasonic3_Ping(void);
uint8_t YEEP144_Ultrasonic4_Ping(void);

uint16_t YEEP144_Ultrasonic1_PingBlocking(void);
uint16_t YEEP144_Ultrasonic2_PingBlocking(void);
uint16_t YEEP144_Ultrasonic3_PingBlocking(void);
uint16_t YEEP144_Ultrasonic4_PingBlocking(void);

void YEEP144_Ultrasonic_EXTI10Handler(void);
void YEEP144_Ultrasonic_EXTI12Handler(void);

extern uint16_t ultrasonicTickLen[4];

#endif