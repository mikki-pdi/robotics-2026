#ifndef __YEEP144_SSD1306_H
#define __YEEP144_SSD1306_H

#define SSD1306_BLACK 0  
#define SSD1306_WHITE 1  
#define SSD1306_INVERSE 2

#include <stdbool.h>
#include <stdarg.h>

void YEEP144_SSD1306_Init(void);
void YEEP144_SSD1306_Display(void);
void YEEP144_SSD1306_ClearDisplay(void);
void YEEP144_SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t colour);
bool YEEP144_SSD1306_GetPixel(uint8_t x, uint8_t y);

void YEEP144_SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t colour);

void YEEP144_SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colour);
void YEEP144_SSD1306_DrawFillRect(uint8_t x, uint8_t y, int8_t w, int8_t h, uint8_t colour);

void YEEP144_SSD1306_DrawChar(uint8_t x, uint8_t y, uint8_t c, uint8_t colour, uint8_t bg, uint8_t size);
int YEEP144_SSD1306_printf(const char * format, ...);
void YEEP144_SSD1306_SetTextSize(uint8_t size);
void YEEP144_SSD1306_SetTextCursor(uint8_t x, uint8_t y);
void YEEP144_SSD1306_SetTextWrap(uint8_t wrap);

void YEEP144_SSD1306_DrawBitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint8_t w, uint8_t h);

void YEEP144_SSD1306_InvertDisplay(bool i);
void YEEP144_SSD1306_SendCommand(uint8_t command);

extern const unsigned char UOYLogo[];

#endif