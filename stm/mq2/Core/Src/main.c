#include "stm32f4xx.h"
#include <stdio.h>
#include <math.h>

// Pin and Port Definitions
#define MQ2_PIN_PORT    GPIOA
#define MQ2_PIN         0

#define LED_GREEN_PORT  GPIOA
#define LED_GREEN_PIN   4
#define LED_YELLOW_PORT GPIOA
#define LED_YELLOW_PIN  5
#define LED_RED_PORT    GPIOA
#define LED_RED_PIN     6
#define LED_STOP_PORT   GPIOA
#define LED_STOP_PIN    7

#define GAS_VALVE_PORT  GPIOA
#define GAS_VALVE_PIN   1

#define PB0_PORT        GPIOB
#define PB0_PIN         0
#define PB1_PORT        GPIOB
#define PB1_PIN         1

#define BUZZER_PWM_PORT GPIOA
#define BUZZER_PWM_PIN  8

// Threshold and Parameter Definitions
#define GAS_THRESHOLD_LOW   300
#define GAS_THRESHOLD_MID   900
#define GAS_THRESHOLD_HIGH  1600
#define GAS_THRESHOLD_BUZZER 1200
#define GAS_THRESHOLD_VALVE  1000

#define PI 3.14159265
#define SAMPLES 100
#define VREF 3.3f
#define ADC_MAX 4095.0f

#define BUTTON_DEBOUNCE_TIME 20 // Debounce time in 10ms ticks (20 * 10ms = 200ms)

#define OLED_I2C_ADDRESS 0x3C

// Volatile Global Variables
volatile uint16_t pwm_table[SAMPLES];
volatile uint32_t current_index = 0;
volatile uint8_t buzzer_active = 0;

volatile uint32_t timer_counter = 0; // Increments every 10ms in TIM2 ISR
volatile uint32_t led_red_counter = 0;
volatile uint8_t led_red_frequency = 1;
volatile uint16_t current_gas_value = 0;

volatile uint8_t stop_mode_enabled = 0;
volatile uint32_t stop_mode_timer = 0;
volatile uint8_t stop_mode_protection = 0;

volatile uint8_t reset_protection_active = 0;
volatile uint32_t reset_protection_timer = 0;

volatile uint8_t gas_valve_state = 0;

volatile uint32_t oled_update_counter = 0;
volatile uint8_t oled_need_update = 0;

// New variables for interrupt-based debouncing
volatile uint32_t last_pb0_press_time = 0;
volatile uint32_t last_pb1_press_time = 0;


const uint8_t font5x7[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space ' '
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // colon ':'
    {0x00, 0x00, 0x00, 0x00, 0x00}  // dash '-'
};

// Function Prototypes
void UART2_Init(void);
void UART2_SendChar(uint8_t c);
void UART2_SendString(char *str);
void ADC1_Init(void);
uint16_t ADC1_Read(void);
void LED_Init(void);
void Gas_Valve_Init(void);
void TIM2_Init(void);
void Button_Interrupt_Init(void);
void LED_Control(uint16_t gasValue);
void Gas_Valve_Control(uint16_t gasValue);
void Send_Gas_Value(uint16_t gasValue);
uint8_t Calculate_Red_LED_Frequency(uint16_t gasValue);
void Enter_Stop_Mode(void);
void Exit_Stop_Mode(void);
void System_Reset(void);

void delay_ms(uint32_t ms);
void make_triangle_sin_table(void);
void init_pwm(void);
void update_pwm(void);

void I2C1_Init(void);
void I2C_Write(uint8_t address, uint8_t *data, uint8_t size);
void SSD1306_Command(uint8_t cmd);
void SSD1306_Data(uint8_t data);
void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_SetCursor(uint8_t col, uint8_t page);
void SSD1306_PrintChar(const uint8_t *chr);
void SSD1306_PrintString(char *str);
void SSD1306_PrintNumber(uint16_t num);
void SSD1306_UpdateDisplay(void);

int main(void) {
    SystemCoreClockUpdate();
    UART2_Init();
    ADC1_Init();
    LED_Init();
    Gas_Valve_Init();
    TIM2_Init();
    init_pwm();
    make_triangle_sin_table();
    Button_Interrupt_Init(); // Use new interrupt-based init
    I2C1_Init();
    SSD1306_Init();

    SSD1306_Clear();
    SSD1306_SetCursor(10, 3);
    SSD1306_PrintString("SYSTEM STARTING");
    delay_ms(2000); // Display startup message

    reset_protection_active = 1; // Enable reset protection for 1 sec at startup
    reset_protection_timer = 0;

    uint16_t gasValue;

    while (1) {
        if (!stop_mode_enabled) {
            gasValue = ADC1_Read();
            current_gas_value = gasValue;
            LED_Control(gasValue);
            Gas_Valve_Control(gasValue);
            Send_Gas_Value(gasValue);

            if (gasValue >= GAS_THRESHOLD_BUZZER) {
                buzzer_active = 1;
                update_pwm();
            } else {
                buzzer_active = 0;
                update_pwm();
            }
        }

        if (oled_need_update) {
            SSD1306_UpdateDisplay();
            oled_need_update = 0;
        }

        // Button handling is now done in ISRs, no polling needed here
        delay_ms(10);
    }
}

// --- Initialization Functions ---

void UART2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    // PA2 (TX), PA3 (RX) to AF7
    GPIOA->MODER &= ~((3U << (2*2)) | (3U << (3*2)));
    GPIOA->MODER |= (2U << (2*2)) | (2U << (3*2));
    GPIOA->AFR[0] |= (7U << (2*4)) | (7U << (3*4));
    // Assuming 16MHz APB1 clock
    USART2->BRR = 16000000 / 9600;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void ADC1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    MQ2_PIN_PORT->MODER |= (3 << (MQ2_PIN * 2)); // Analog mode for PA0
    ADC1->SMPR2 |= (7 << (3 * 0)); // Max sampling time for channel 0
    ADC1->SQR1 = 0; // 1 conversion
    ADC1->SQR3 = 0; // Channel 0 is the first conversion
    ADC1->CR2 |= ADC_CR2_ADON; // Turn on ADC
    delay_ms(1); // Wait for ADC to stabilize
}

void LED_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~((3U << (LED_GREEN_PIN * 2)) | (3U << (LED_YELLOW_PIN * 2)) | (3U << (LED_RED_PIN * 2)) | (3U << (LED_STOP_PIN * 2)));
    GPIOA->MODER |= (1U << (LED_GREEN_PIN * 2)) | (1U << (LED_YELLOW_PIN * 2)) | (1U << (LED_RED_PIN * 2)) | (1U << (LED_STOP_PIN * 2));
    GPIOA->BSRR = (1U << (LED_GREEN_PIN + 16)) | (1U << (LED_YELLOW_PIN + 16)) | (1U << (LED_RED_PIN + 16)) | (1U << (LED_STOP_PIN + 16));
}

void Gas_Valve_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (GAS_VALVE_PIN * 2));
    GPIOA->MODER |= (1U << (GAS_VALVE_PIN * 2));
    GPIOA->BSRR = (1U << (GAS_VALVE_PIN + 16)); // Initially closed (off)
    gas_valve_state = 0;
}

void TIM2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 1600 - 1;  // 16MHz / 1600 = 10kHz
    TIM2->ARR = 100 - 1;   // 100 / 10kHz = 10ms interrupt
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC_SetPriority(TIM2_IRQn, 2); // Lower priority than buttons
    NVIC_EnableIRQ(TIM2_IRQn);
}

void Button_Interrupt_Init(void) {
    // 1. Enable clocks for GPIOB and SYSCFG
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // 2. Configure PB0 and PB1 as input with pull-down
    PB0_PORT->MODER &= ~((3U << (PB0_PIN * 2)) | (3U << (PB1_PIN * 2)));
    PB0_PORT->PUPDR &= ~((3U << (PB0_PIN * 2)) | (3U << (PB1_PIN * 2))); // Clear
    PB0_PORT->PUPDR |= (2U << (PB0_PIN * 2)) | (2U << (PB1_PIN * 2));   // Set pull-down

    // 3. Connect EXTI Line to GPIO Pin
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0 | SYSCFG_EXTICR1_EXTI1); // Clear
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB | SYSCFG_EXTICR1_EXTI1_PB; // Connect PB0->EXTI0, PB1->EXTI1

    // 4. Configure EXTI Controller
    EXTI->IMR |= (1U << PB0_PIN) | (1U << PB1_PIN);    // Unmask EXTI0 and EXTI1
    EXTI->RTSR |= (1U << PB0_PIN) | (1U << PB1_PIN);   // Enable Rising edge trigger
    EXTI->FTSR &= ~((1U << PB0_PIN) | (1U << PB1_PIN)); // Disable Falling edge trigger

    // 5. Enable and set priority in NVIC
    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_EnableIRQ(EXTI0_IRQn);

    NVIC_SetPriority(EXTI1_IRQn, 1);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

void init_pwm() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    // PA8 as AF1 (TIM1_CH1)
    BUZZER_PWM_PORT->MODER &= ~(3U << (BUZZER_PWM_PIN * 2));
    BUZZER_PWM_PORT->MODER |= (2U << (BUZZER_PWM_PIN * 2));
    BUZZER_PWM_PORT->AFR[1] &= ~(0xFU << ((BUZZER_PWM_PIN - 8) * 4));
    BUZZER_PWM_PORT->AFR[1] |= (1U << ((BUZZER_PWM_PIN - 8) * 4));

    // TIM1 Configuration
    TIM1->PSC = 16 - 1;  // 16MHz / 16 = 1MHz timer clock
    TIM1->ARR = 999;     // 1MHz / (999+1) = 1kHz PWM frequency
    TIM1->CCR1 = 0;      // Initial duty cycle 0%

    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload
    TIM1->CCER |= TIM_CCER_CC1E;    // Enable channel 1 output
    TIM1->BDTR |= TIM_BDTR_MOE;     // Main output enable
    TIM1->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; // Auto-reload preload enable, Counter enable
    TIM1->EGR |= TIM_EGR_UG; // Generate update event to load registers
}

void I2C1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Configure PB8 (SCL) and PB9 (SDA)
    GPIOB->MODER &= ~((3U << (8 * 2)) | (3U << (9 * 2)));
    GPIOB->MODER |= (2U << (8 * 2)) | (2U << (9 * 2));
    GPIOB->OTYPER |= (1U << 8) | (1U << 9);
    GPIOB->OSPEEDR |= (3U << (8 * 2)) | (3U << (9 * 2));
    GPIOB->PUPDR &= ~((3U << (8 * 2)) | (3U << (9 * 2)));

    // Set Alternate Function to AF4 for I2C1
    GPIOB->AFR[1] &= ~((0xFU << ((8 - 8) * 4)) | (0xFU << ((9 - 8) * 4)));
    GPIOB->AFR[1] |= (4U << ((8 - 8) * 4)) | (4U << ((9 - 8) * 4));

    // Configure I2C1
    I2C1->CR1 |= I2C_CR1_SWRST; // Reset I2C
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 = 16;   // APB1 frequency in MHz (16MHz)
    I2C1->CCR = 80;   // Standard mode 100kHz: 16MHz / (2 * 100kHz) = 80
    I2C1->TRISE = 17; // 1000ns / (1/16MHz) + 1 = 17
    I2C1->CR1 |= I2C_CR1_PE; // Enable I2C peripheral
}

void SSD1306_Init(void) {
    delay_ms(100);
    SSD1306_Command(0xAE); // Display OFF
    SSD1306_Command(0xD5); SSD1306_Command(0x80); // Set Clock Divide Ratio
    SSD1306_Command(0xA8); SSD1306_Command(0x3F); // Set MUX Ratio (64)
    SSD1306_Command(0xD3); SSD1306_Command(0x00); // Set Display Offset
    SSD1306_Command(0x40); // Set Start Line
    SSD1306_Command(0x8D); SSD1306_Command(0x14); // Enable Charge Pump
    SSD1306_Command(0x20); SSD1306_Command(0x00); // Memory Addressing Mode (Horizontal)
    SSD1306_Command(0xA1); // Segment Remap (column 127 is mapped to SEG0)
    SSD1306_Command(0xC8); // COM Output Scan Direction (reversed)
    SSD1306_Command(0xDA); SSD1306_Command(0x12); // Set COM Pins
    SSD1306_Command(0x81); SSD1306_Command(0xCF); // Set Contrast
    SSD1306_Command(0xD9); SSD1306_Command(0xF1); // Set Pre-charge Period
    SSD1306_Command(0xDB); SSD1306_Command(0x40); // Set VCOMH Deselect Level
    SSD1306_Command(0xA4); // Entire Display ON from RAM
    SSD1306_Command(0xA6); // Normal Display
    SSD1306_Command(0xAF); // Display ON
}

// --- Core Logic Functions ---

uint16_t ADC1_Read(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return (uint16_t)ADC1->DR;
}

void LED_Control(uint16_t gasValue) {
    GPIOA->BSRR = (1U << (LED_GREEN_PIN + 16)) | (1U << (LED_YELLOW_PIN + 16));
    if (gasValue < GAS_THRESHOLD_LOW) {
        GPIOA->BSRR = (1U << LED_GREEN_PIN);
        led_red_frequency = 0;
        GPIOA->BSRR = (1U << (LED_RED_PIN + 16)); // Turn off RED LED
    } else if (gasValue < GAS_THRESHOLD_MID) {
        GPIOA->BSRR = (1U << LED_YELLOW_PIN);
        led_red_frequency = 0;
        GPIOA->BSRR = (1U << (LED_RED_PIN + 16)); // Turn off RED LED
    } else {
        led_red_frequency = Calculate_Red_LED_Frequency(gasValue);
    }
}

void Gas_Valve_Control(uint16_t gasValue) {
    if (gasValue > GAS_THRESHOLD_VALVE) {
        if (gas_valve_state == 0) { // If valve is open (state 0)
            GPIOA->BSRR = (1U << GAS_VALVE_PIN); // Close it (turn on relay)
            gas_valve_state = 1; // Update state to closed
        }
    } else {
        if (gas_valve_state == 1) { // If valve is closed (state 1)
            GPIOA->BSRR = (1U << (GAS_VALVE_PIN + 16)); // Open it (turn off relay)
            gas_valve_state = 0; // Update state to open
        }
    }
}

uint8_t Calculate_Red_LED_Frequency(uint16_t gasValue) {
    if (gasValue < GAS_THRESHOLD_MID) return 0;
    if (gasValue >= GAS_THRESHOLD_HIGH) return 10; // Max frequency (10 Hz)

    // Linear scaling from 1Hz to 10Hz
    uint16_t range = GAS_THRESHOLD_HIGH - GAS_THRESHOLD_MID;
    uint16_t step = range / 9; // 9 steps for 1-10Hz
    uint8_t frequency = ((gasValue - GAS_THRESHOLD_MID) / step) + 1;
    return (frequency > 10) ? 10 : frequency;
}

void Enter_Stop_Mode(void) {
    stop_mode_enabled = 1;
    stop_mode_protection = 1; // Enable 1s protection against immediate exit
    stop_mode_timer = 0;
    UART2_SendString("000000\r\n");

    // Turn off all status LEDs and buzzer
    GPIOA->BSRR = (1U << (LED_GREEN_PIN + 16)) | (1U << (LED_YELLOW_PIN + 16)) | (1U << (LED_RED_PIN + 16));
    buzzer_active = 0;
    update_pwm();

    // Turn on STOP LED
    GPIOA->BSRR = (1U << LED_STOP_PIN);

    // Ensure gas valve is open (safe state)
    GPIOA->BSRR = (1U << (GAS_VALVE_PIN + 16));
    gas_valve_state = 0;
}

void Exit_Stop_Mode(void) {
    stop_mode_enabled = 0;
    UART2_SendString("111111\r\n");
    // Turn off STOP LED
    GPIOA->BSRR = (1U << (LED_STOP_PIN + 16));
}

void System_Reset(void) {
    UART2_SendString("222222\r\n111111\r\n");
    delay_ms(100);
    NVIC_SystemReset();
}

void make_triangle_sin_table(void) {
    for (int i = 0; i < SAMPLES; i++) {
        float phase = (float)i / SAMPLES;
        float triangle = phase < 0.5f ? (2.0f * phase) : (2.0f * (1.0f - phase));
        float sin_mod = (sinf(2 * PI * phase) + 1.0f) / 2.0f;
        // Scale to ARR value (999)
        pwm_table[i] = (uint16_t)(triangle * sin_mod * 999);
    }
}

void update_pwm(void) {
    if (buzzer_active == 1) {
        TIM1->CCR1 = pwm_table[current_index];
        current_index = (current_index + 1) % SAMPLES;
    } else {
        TIM1->CCR1 = 0;
    }
}


// --- Communication & Display Functions ---

void UART2_SendChar(uint8_t c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void UART2_SendString(char *str) {
    while (*str) UART2_SendChar(*str++);
}

void Send_Gas_Value(uint16_t gasValue) {
    char buffer[20];
    sprintf(buffer, "Gas: %d\r\n", gasValue);
    UART2_SendString(buffer);
}

void I2C_Write(uint8_t address, uint8_t* data, uint8_t size) {
    while(I2C1->SR2 & I2C_SR2_BUSY);
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = address << 1; // Address + Write bit
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Clear ADDR flag
    for(uint8_t i = 0; i < size; i++) {
        while(!(I2C1->SR1 & I2C_SR1_TXE));
        I2C1->DR = data[i];
    }
    while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

void SSD1306_Command(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd}; // 0x00 for Co=0, D/C#=0 (command)
    I2C_Write(OLED_I2C_ADDRESS, data, 2);
}

void SSD1306_Data(uint8_t data_byte) {
    uint8_t data[2] = {0x40, data_byte}; // 0x40 for Co=0, D/C#=1 (data)
    I2C_Write(OLED_I2C_ADDRESS, data, 2);
}

void SSD1306_Clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        SSD1306_SetCursor(0, page);
        for (uint8_t col = 0; col < 128; col++) {
            SSD1306_Data(0x00);
        }
    }
}

void SSD1306_SetCursor(uint8_t col, uint8_t page) {
    SSD1306_Command(0xB0 + page);
    SSD1306_Command(0x00 + (col & 0x0F));
    SSD1306_Command(0x10 + ((col >> 4) & 0x0F));
}

void SSD1306_PrintChar(const uint8_t *chr) {
    for (int i = 0; i < 5; i++) {
        SSD1306_Data(chr[i]);
    }
    SSD1306_Data(0x00); // Spacer
}

void SSD1306_PrintString(char *str) {
    while (*str) {
        if (*str >= '0' && *str <= '9') SSD1306_PrintChar(font5x7[*str - '0']);
        else if (*str >= 'A' && *str <= 'Z') SSD1306_PrintChar(font5x7[*str - 'A' + 10]);
        else if (*str == ' ') SSD1306_PrintChar(font5x7[36]);
        else if (*str == ':') SSD1306_PrintChar(font5x7[37]);
        else if (*str == '-') SSD1306_PrintChar(font5x7[38]);
        str++;
    }
}

void SSD1306_PrintNumber(uint16_t num) {
    char buffer[6];
    sprintf(buffer, "%5d", num); // Use %5d for right alignment
    SSD1306_PrintString(buffer);
}

void SSD1306_UpdateDisplay(void) {
    SSD1306_SetCursor(0, 0);
    SSD1306_PrintString("GAS LVL: ");
    SSD1306_PrintNumber(current_gas_value);

    SSD1306_SetCursor(0, 2);
    SSD1306_PrintString("STATUS: ");
    if (stop_mode_enabled) {
        SSD1306_PrintString("STOPPED  ");
    } else {
        if (current_gas_value < GAS_THRESHOLD_LOW) SSD1306_PrintString("SAFE     ");
        else if (current_gas_value < GAS_THRESHOLD_MID) SSD1306_PrintString("CAUTION  ");
        else SSD1306_PrintString("DANGER!  ");
    }

    SSD1306_SetCursor(0, 4);
    SSD1306_PrintString("VALVE: ");
    if (gas_valve_state == 0) SSD1306_PrintString("OPEN     ");
    else SSD1306_PrintString("CLOSED   ");

    SSD1306_SetCursor(0, 6);
    SSD1306_PrintString("SYSTEM: ");
    if (stop_mode_enabled) SSD1306_PrintString("HALTED   ");
    else SSD1306_PrintString("RUNNING  ");
}

// --- Utility and Interrupt Service Routines ---

void delay_ms(uint32_t ms) {
    // Note: A SysTick-based delay is better, but this is simple for demonstration.
    // It assumes a 16MHz internal HSI clock.
    volatile uint32_t count = 1600 * ms; // Approximation
    while(count--);
}

// ISR for TIM2 (10ms periodic interrupt)
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;

        timer_counter++;
        oled_update_counter++;

        // Trigger OLED update every 50 ticks (500ms)
        if (oled_update_counter >= 50) {
            oled_need_update = 1;
            oled_update_counter = 0;
        }

        // Handle protection timers
        if (stop_mode_protection) {
            stop_mode_timer++;
            if (stop_mode_timer >= 100) { // 1 second protection
                stop_mode_protection = 0;
            }
        }
        if (reset_protection_active) {
            reset_protection_timer++;
            if (reset_protection_timer >= 100) { // 1 second protection
                reset_protection_active = 0;
            }
        }

        // Blink RED LED logic
        if (!stop_mode_enabled && led_red_frequency > 0) {
            led_red_counter++;
            // Period is 100 ticks (1s) / frequency
            uint32_t led_red_period = 100 / (led_red_frequency * 2);
            if (led_red_counter >= led_red_period) {
                GPIOA->ODR ^= (1U << LED_RED_PIN);
                led_red_counter = 0;
            }
        } else if (stop_mode_enabled) {
            GPIOA->BSRR = (1U << (LED_RED_PIN + 16)); // Ensure red LED is off in stop mode
            led_red_counter = 0;
        }
    }
}

// ISR for PB0 (Reset Button)
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1U << PB0_PIN)) {
        EXTI->PR = (1U << PB0_PIN); // Clear pending bit

        // Debounce check
        if ((timer_counter - last_pb0_press_time) > BUTTON_DEBOUNCE_TIME) {
            if (!reset_protection_active) {
                System_Reset();
            }
            last_pb0_press_time = timer_counter;
        }
    }
}

// ISR for PB1 (Stop/Start Button)
void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1U << PB1_PIN)) {
        EXTI->PR = (1U << PB1_PIN); // Clear pending bit

        // Debounce check
        if ((timer_counter - last_pb1_press_time) > BUTTON_DEBOUNCE_TIME) {
            if (!stop_mode_enabled) {
                Enter_Stop_Mode();
            } else if (!stop_mode_protection) {
                Exit_Stop_Mode();
            }
            last_pb1_press_time = timer_counter;
        }
    }
}
