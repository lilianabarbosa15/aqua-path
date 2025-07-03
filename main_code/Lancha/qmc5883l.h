#ifndef QMC5883L_H
#define QMC5883L_H

#include "hardware/i2c.h"

#define QMC5883L_ADDR        0x0D

#define QMC5883L_REG_X_LSB   0x00
#define QMC5883L_REG_CTRL1   0x09
#define QMC5883L_REG_RESET   0x0A

void qmc5883l_init(i2c_inst_t *i2c);
bool qmc5883l_read_raw(i2c_inst_t *i2c, int16_t *x, int16_t *y, int16_t *z);

#endif
