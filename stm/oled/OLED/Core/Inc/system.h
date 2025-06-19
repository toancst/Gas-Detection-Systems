// ====== system.h ======
#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

extern volatile uint32_t system_tick;

void SysTick_Init(void);
void SysTick_Handler(void);
void Delay_ms(uint32_t ms);
uint32_t GetTick(void);

#endif
