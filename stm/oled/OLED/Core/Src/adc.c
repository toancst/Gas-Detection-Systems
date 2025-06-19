// ====== adc.c ======
#include "stm32f4xx.h"
#include "adc.h"
#include "system.h"

void ADC_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER |= (3 << (0 * 2));

    ADC1->SQR3 = 0;
    ADC1->SMPR2 |= (7 << 0);
    ADC1->CR2 |= ADC_CR2_ADON;

    Delay_ms(2);
}

uint16_t ADC_Read(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

uint8_t Mode_Update_From_ADC(void) {
    uint16_t adc_value = ADC_Read();
    if (adc_value < 200) return 0;
    else if (adc_value < 1365) return 1;
    else if (adc_value < 2730) return 2;
    else return 3;
}

