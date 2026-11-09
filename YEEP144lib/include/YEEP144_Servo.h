#ifndef __YEEP144_SERVO_H
#define __YEEP144_SERVO_H

void YEEP144_Servo_Init1_2(void);
void YEEP144_Servo_Init3(void);
void YEEP144_Servo_Init4(void);

void YEEP144_Servo1_SetDirect(uint16_t pulse);
void YEEP144_Servo2_SetDirect(uint16_t pulse);
void YEEP144_Servo3_SetDirect(uint16_t pulse);
void YEEP144_Servo4_SetDirect(uint16_t pulse);

void YEEP144_Servo1_SetAngle(float angle);
void YEEP144_Servo2_SetAngle(float angle);
void YEEP144_Servo3_SetAngle(float angle);
void YEEP144_Servo4_SetAngle(float angle);

void YEEP144_Servo_SetMinTicks(uint8_t servo, uint16_t min);
void YEEP144_Servo_SetMaxTicks(uint8_t servo, uint16_t max);

void YEEP144_Servo_SetMinAngle(uint8_t servo, float min);
void YEEP144_Servo_SetMaxAngle(uint8_t servo, float max);

#endif