// ====== adc.h ======
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void ADC_Init(void);
uint16_t ADC_Read(void);
uint8_t Mode_Update_From_ADC(void);

#endif
