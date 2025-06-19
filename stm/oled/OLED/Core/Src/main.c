 #include "stm32f4xx.h"
#include "system.h"
#include "i2c.h"
#include "oled.h"
#include "adc.h"
#include "pwm.h"
#include "led.h"
#include "exti.h"

extern volatile uint8_t mode;
extern volatile uint8_t countdown;
extern volatile uint8_t button_pressed;

int main(void) {
    SysTick_Init();
    I2C1_Init();
    ADC_Init();
    PWM_Init();
    LED_Init();
    GPIO_EXTI_Init();


    if (!SSD1306_Init()) {
        while (1) {
            GPIOA->ODR ^= (1 << 1);
            Delay_ms(200);
        }
    }

    // Hiển thị khởi động
    SSD1306_Clear();
    SSD1306_PrintTextCentered(3, "SYSTEM READY");
    Delay_ms(2000);
    oled_state = 3; // Chuyển sang chế độ hoạt động

    uint32_t last_update = 0;
    uint32_t last_countdown = 0;
    uint32_t last_display = 0;

    while (1) {
        uint32_t current_time = GetTick();

        // ✅ Luôn cho phép hiển thị OLED dù system_active = 0
        if ((current_time - last_display) >= 500) {
            switch (oled_state) {
                case 0:
                    SSD1306_Clear();
                    SSD1306_PrintTextCentered(3, "SYSTEM READY");
                    break;
                case 1:
                    SSD1306_DisplayStatus(mode, countdown);
                    break;
                case 2:
                    SSD1306_Clear();
                    SSD1306_PrintTextCentered(3, "SYSTEM STOPPED");
                    break;
                case 3:
                    SSD1306_Clear();
                    SSD1306_PrintTextCentered(2, "TIME: INF");

                    char mode_str[16];
                    sprintf(mode_str, "MODE: %d", mode);
                    SSD1306_PrintTextCentered(4, mode_str);
                    break;
            }
            last_display = current_time;
        }

        // ❌ Chặn toàn bộ xử lý bên dưới nếu đang STOPPED
        if (!system_active) {
            Delay_ms(10);
            continue;
        }

        // Cập nhật mode + PWM + LED mỗi 100ms
        if ((current_time - last_update) >= 100) {
            if (!button_pressed) {
                mode = Mode_Update_From_ADC();
            } else {
                button_pressed = 0;
            }

            if (oled_state == 1 || oled_state == 3) {
                Update_PWM_From_Mode(mode);
                LED_Update(mode);
            } else {
                TIM4->CCR2 = 0;
                LED_Update(0);
            }

            last_update = current_time;
        }

        // Xử lý countdown độc lập mỗi 1000ms
        if ((current_time - last_countdown) >= 1000) {
            if (oled_state == 1 && countdown > 0 && mode != 0) {
                countdown--;
                if (countdown == 0) {
                    oled_state = 0; // Trở lại trạng thái READY
                }
            }
            last_countdown = current_time;
        }

        Delay_ms(10);
    }
}
