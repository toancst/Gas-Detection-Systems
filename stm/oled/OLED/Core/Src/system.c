// ====== system.c ======
#include "stm32f4xx.h"
#include "system.h"

volatile uint32_t system_tick = 0;

void SysTick_Init(void) {
    SysTick->LOAD = (16000000 / 1000) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {
    system_tick++;
}

void Delay_ms(uint32_t ms) {
    uint32_t start = system_tick;
    while ((system_tick - start) < ms);
}

uint32_t GetTick(void) {
    return system_tick;
}
