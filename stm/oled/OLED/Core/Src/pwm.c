// ====== pwm.c ======
#include "stm32f4xx.h"
#include "pwm.h"

void PWM_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    GPIOB->MODER &= ~(3 << (7 * 2));
    GPIOB->MODER |= (2 << (7 * 2));
    GPIOB->AFR[0] |= (2 << (7 * 4));

    TIM4->PSC = 1599;
    TIM4->ARR = 100;
    TIM4->CCR2 = 0;

    TIM4->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM4->CCMR1 |= (6 << TIM_CCMR1_OC2M_Pos);
    TIM4->CCMR1 |= TIM_CCMR1_OC2PE;
    TIM4->CCER |= TIM_CCER_CC2E;
    TIM4->CR1 |= TIM_CR1_CEN;
}

void Update_PWM_From_Mode(uint8_t mode) {
    switch (mode) {
        case 3: TIM4->CCR2 = 100; break;  // 100% tốc độ
        case 2: TIM4->CCR2 = 70;  break;
        case 1: TIM4->CCR2 = 40;  break;
        case 0: TIM4->CCR2 = 0;   break;  // Dừng
        default: TIM4->CCR2 = 0;  break;
    }
}
