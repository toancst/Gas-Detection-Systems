// ====== i2c.c ======
#include "stm32f4xx.h"
#include "i2c.h"

#define I2C_TIMEOUT 100000

void I2C1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~(0xF << (8*2));
    GPIOB->MODER |=  (0xA << (8*2));
    GPIOB->OTYPER |= (0x3 << 8);
    GPIOB->OSPEEDR |= (0xF << (8*2));
    GPIOB->PUPDR |= (0x5 << (8*2));
    GPIOB->AFR[1] |= (0x44 << ((8-8)*4));

    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR2 = 16;
    I2C1->CCR = 80;
    I2C1->TRISE = 17;
    I2C1->CR1 |= I2C_CR1_PE;
}

uint8_t I2C_WriteByte(uint8_t addr, uint8_t reg, uint8_t data) {
    uint32_t timeout;
    I2C1->CR1 |= I2C_CR1_START;
    timeout = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_SB) && --timeout);
    if (!timeout) return 0;

    I2C1->DR = addr << 1;
    timeout = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_ADDR) && --timeout);
    if (!timeout) return 0;
    (void)I2C1->SR2;

    timeout = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_TXE) && --timeout);
    if (!timeout) return 0;
    I2C1->DR = reg;

    timeout = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_TXE) && --timeout);
    if (!timeout) return 0;
    I2C1->DR = data;

    timeout = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_BTF) && --timeout);
    if (!timeout) return 0;
    I2C1->CR1 |= I2C_CR1_STOP;

    return 1;
}
