// ====== i2c.h ======
#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void I2C1_Init(void);
uint8_t I2C_WriteByte(uint8_t addr, uint8_t reg, uint8_t data);

#endif
