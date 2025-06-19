// ====== oled.h ======
#ifndef OLED_H
#define OLED_H

#include <stdint.h>

uint8_t SSD1306_Init(void);
void SSD1306_SetCursor(uint8_t col, uint8_t page);
void SSD1306_Clear(void);
void SSD1306_PrintChar(char ch);
void SSD1306_PrintTextCentered(uint8_t page, const char* str);
void SSD1306_DisplayStatus(uint8_t current_mode, uint8_t seconds_left);

#endif
