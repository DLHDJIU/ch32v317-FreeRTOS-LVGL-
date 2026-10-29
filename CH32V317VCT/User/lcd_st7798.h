#ifndef __LCD_ST7798_H
#define __LCD_ST7798_H

#include "debug.h"

#define LCD_ST7798_WIDTH   320u
#define LCD_ST7798_HEIGHT  240u

void LCD_ST7798_Init(void);
void LCD_ST7798_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_ST7798_WritePixels(const uint16_t *pixels, uint32_t count);
void LCD_ST7798_Fill(uint16_t rgb565);
void LCD_ST7798_Backlight(uint8_t on);
void LCD_ST7798_SetBacklight(uint8_t percent);

#endif
