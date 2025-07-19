#include "bmi160_lib.h"
#include "pico/stdlib.h"

// -------------------- BMI160 Registers --------------------
#define BMI160_REG_CHIP_ID      0x00
#define BMI160_REG_DATA_GYR     0x0C  // 6 bytes
#define BMI160_REG_DATA_ACC     0x12  // 6 bytes
#define BMI160_REG_CMD          0x7E
#define BMI160_REG_ACC_RANGE    0x41
#define BMI160_REG_GYR_RANGE    0x43
#define BMI160_CMD_ACC_NORMAL   0x11
#define BMI160_CMD_GYR_NORMAL   0x15
#define BMI160_CMD_SOFT_RESET   0xB6

static inline bool reg_write(i2c_inst_t *i2c, uint8_t addr,
                              uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_write_blocking(i2c, addr, buf, 2, false) == 2;
}

static inline bool reg_read(i2c_inst_t *i2c, uint8_t addr,
                             uint8_t reg, uint8_t *buf, size_t len)
{
    return (i2c_write_blocking(i2c, addr, &reg, 1, true) == 1) &&
           (i2c_read_blocking(i2c, addr, buf, len, false) == (int)len);
}

bool bmi160_init(i2c_inst_t *i2c,
                 uint sda_pin, uint scl_pin,
                 uint8_t addr)
{
    // Configure GPIO + I2C
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    sleep_ms(10);

    // Soft reset
    reg_write(i2c, addr, BMI160_REG_CMD, BMI160_CMD_SOFT_RESET);
    sleep_ms(100);

    // Check CHIP_ID (should be 0xD1)
    uint8_t chip_id;
    if (!reg_read(i2c, addr, BMI160_REG_CHIP_ID, &chip_id, 1) ||
        chip_id != 0xD1)
        return false;

    // Range ±2 g (0x03) and ±2000°/s (0x00)
    reg_write(i2c, addr, BMI160_REG_ACC_RANGE, 0x03);
    reg_write(i2c, addr, BMI160_REG_GYR_RANGE, 0x00);

    // Enable normal modes
    reg_write(i2c, addr, BMI160_REG_CMD, BMI160_CMD_ACC_NORMAL);
    reg_write(i2c, addr, BMI160_REG_CMD, BMI160_CMD_GYR_NORMAL);
    sleep_ms(50);

    return true;
}

bool bmi160_read(i2c_inst_t *i2c,
                 uint8_t addr,
                 bmi160_data_t *out)
{
    uint8_t raw[12];

    // Read Gyro(6) + Acc(6) in one go
    if (!reg_read(i2c, addr, BMI160_REG_DATA_GYR, raw, 12))
        return false;

    int16_t gx = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t gy = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t gz = (int16_t)((raw[5] << 8) | raw[4]);

    int16_t ax = (int16_t)((raw[7] << 8) | raw[6]);
    int16_t ay = (int16_t)((raw[9] << 8) | raw[8]);
    int16_t az = (int16_t)((raw[11] << 8) | raw[10]);

    out->gx_dps = (float)gx / BMI160_GYR_LSB_PER_DPS;
    out->gy_dps = (float)gy / BMI160_GYR_LSB_PER_DPS;
    out->gz_dps = (float)gz / BMI160_GYR_LSB_PER_DPS;

    out->ax_g = (float)ax / BMI160_ACC_LSB_PER_G;
    out->ay_g = (float)ay / BMI160_ACC_LSB_PER_G;
    out->az_g = (float)az / BMI160_ACC_LSB_PER_G;

    return true;
}
