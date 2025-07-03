#include "qmc5883l.h"
#include "pico/stdlib.h"

void qmc5883l_init(i2c_inst_t *i2c) {
    uint8_t config[2];

    // Soft reset
    config[0] = QMC5883L_REG_RESET;
    config[1] = 0x80;
    i2c_write_blocking(i2c, QMC5883L_ADDR, config, 2, false);

    sleep_ms(10);

    // Configure control register:
    // OSR = 512 (00), RNG = 8G (01), ODR = 50Hz (10), MODE = Continuous (01)
    config[0] = QMC5883L_REG_CTRL1;
    config[1] = 0b00011101;  // 0x1D
    i2c_write_blocking(i2c, QMC5883L_ADDR, config, 2, false);
}

bool qmc5883l_read_raw(i2c_inst_t *i2c, int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6];
    uint8_t reg = QMC5883L_REG_X_LSB;

    if (i2c_write_blocking(i2c, QMC5883L_ADDR, &reg, 1, true) < 0)
        return false;

    if (i2c_read_blocking(i2c, QMC5883L_ADDR, buffer, 6, false) < 0)
        return false;

    *x = (int16_t)(buffer[1] << 8 | buffer[0]);
    *y = (int16_t)(buffer[3] << 8 | buffer[2]);
    *z = (int16_t)(buffer[5] << 8 | buffer[4]);

    return true;
}
