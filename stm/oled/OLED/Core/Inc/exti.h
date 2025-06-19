// ====== exti.h ======
#ifndef EXTI_H
#define EXTI_H

#include <stdint.h>

extern volatile uint8_t countdown;
extern volatile uint8_t mode;
extern volatile uint8_t button_pressed;
extern volatile uint8_t system_active;
extern volatile uint8_t oled_state;

void GPIO_EXTI_Init(void);

#endif
