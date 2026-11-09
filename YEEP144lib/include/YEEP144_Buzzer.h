#ifndef __YEEP144_BUZZER_H
#define __YEEP144_BUZZER_H

void YEEP144_Buzzer_Init(void);
void YEEP144_Buzzer_Tone(uint32_t freq, uint32_t duration);
void YEEP144_Buzzer_GoTone(uint32_t freq);
void YEEP144_Buzzer_NoTone(void);
void YEEP144_Buzzer_Daisy(void);

#endif