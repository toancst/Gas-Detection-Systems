// ====== exti.c ======
#include "stm32f4xx.h"
#include "exti.h"
#include "system.h"

volatile uint8_t countdown = 0;
volatile uint8_t mode = 1;
volatile uint8_t button_pressed = 0;
volatile uint8_t system_active = 1; // 1 = hoạt động, 0 = tắt
volatile uint8_t oled_state = 3;

void GPIO_EXTI_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    GPIOA->MODER &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB->MODER &= ~((3 << (0 * 2)) | (3 << (1 * 2)));

    GPIOA->PUPDR |= (1 << (6 * 2)) | (1 << (7 * 2));
    GPIOB->PUPDR |= (1 << (0 * 2)) | (1 << (1 * 2));

    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PA | SYSCFG_EXTICR2_EXTI7_PA;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB | SYSCFG_EXTICR1_EXTI1_PB;

    EXTI->IMR |= (1 << 6) | (1 << 7) | (1 << 0) | (1 << 1);
    EXTI->FTSR |= (1 << 6) | (1 << 7) | (1 << 0) | (1 << 1);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);

    NVIC_SetPriority(EXTI9_5_IRQn, 0); // Ưu tiên cao nhất
    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_SetPriority(EXTI1_IRQn, 1);
}

void EXTI9_5_IRQHandler(void) {
    static uint32_t last_press_time = 0;
    uint32_t current_time = GetTick();

    // Chống rung
    if ((current_time - last_press_time) < 50) {
        EXTI->PR |= (1 << 6) | (1 << 7); // Clear pending
        return;
    }

    // Xử lý nút PA6 (EXTI6): Bật / Tắt hệ thống
    if (EXTI->PR & (1 << 6)) {
        system_active ^= 1;

        if (!system_active) {
            // STOPPED: tắt toàn bộ
            countdown = 0;
            mode = 0;
            oled_state = 2; // SYSTEM STOPPED

            GPIOA->ODR &= ~((1 << 1) | (1 << 2) | (1 << 3)); // Tắt LED
            TIM4->CCR2 = 0; // Tắt PWM

            // ❌ Vô hiệu hóa các ngắt khác
            EXTI->IMR &= ~((1 << 7) | (1 << 0) | (1 << 1));
        } else {
            // Trở lại hoạt động → INFINITE
            countdown = 0;
            mode = 1;
            oled_state = 3; // SYSTEM ACTIVE - INFINITE

            // ✅ Bật lại các ngắt
            EXTI->IMR |= (1 << 7) | (1 << 0) | (1 << 1);
        }

        button_pressed = 1;
        EXTI->PR |= (1 << 6); // Clear pending
    }

    // Xử lý nút PA7 (EXTI7): Chỉ cho phép nếu hệ thống đang hoạt động
    if (EXTI->PR & (1 << 7)) {
        if (system_active) {
            countdown = 10;
            oled_state = 1; // COUNTDOWN mode
            button_pressed = 1;
        }
        EXTI->PR |= (1 << 7); // Clear pending
    }

    last_press_time = current_time;
}


void EXTI0_IRQHandler(void) {
    static uint32_t last_press_time = 0;
    uint32_t current_time = GetTick();
    if ((current_time - last_press_time) > 50) {
        countdown = 20;
        oled_state = 1;
        button_pressed = 1;
        last_press_time = current_time;
    }
    EXTI->PR |= (1 << 0);
}

void EXTI1_IRQHandler(void) {
    static uint32_t last_press_time = 0;
    uint32_t current_time = GetTick();
    if ((current_time - last_press_time) > 50) {
        countdown = 30;
        oled_state = 1;
        button_pressed = 1;
        last_press_time = current_time;
    }
    EXTI->PR |= (1 << 1);
}
