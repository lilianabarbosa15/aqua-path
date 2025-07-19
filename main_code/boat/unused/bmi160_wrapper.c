// bmi160_wrapper.c
#include "bmi160_wrapper.h"
#include "pico/stdlib.h"
#include <math.h>

void bmi160_init(bmi160_ctx_t *ctx) {
    // Initialize I2C and pins
    i2c_init(ctx->i2c, 400 * 1000);
    gpio_set_function(ctx->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(ctx->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(ctx->sda_pin);
    gpio_pull_up(ctx->scl_pin);
    sleep_ms(100);

    // Configure the sensor in normal mode
    uint8_t gyro_cmd[2] = {0x7E, 0x15};  // Gyro normal mode
    i2c_write_blocking(ctx->i2c, ctx->address, gyro_cmd, 2, false);
    sleep_ms(100);

    uint8_t acc_cmd[2] = {0x7E, 0x11};  // Acc normal mode
    i2c_write_blocking(ctx->i2c, ctx->address, acc_cmd, 2, false);
    sleep_ms(100);

    uint8_t gyro_range[2] = {0x43, 0x03};  // ±250 dps
    i2c_write_blocking(ctx->i2c, ctx->address, gyro_range, 2, false);

    uint8_t acc_range[2] = {0x41, 0x03};   // ±2g
    i2c_write_blocking(ctx->i2c, ctx->address, acc_range, 2, false);

    ctx->accumulated_orientation = 0.0f;
    ctx->gyro_z_filtered = 0.0f;
    ctx->last_time = get_absolute_time();
}

// Simulates reading the Z gyroscope
float read_gyro_z(bmi160_ctx_t *ctx) {
    uint8_t reg = 0x10;
    uint8_t data[2];

    i2c_write_blocking(ctx->i2c, ctx->address, &reg, 1, true);
    i2c_read_blocking(ctx->i2c, ctx->address, data, 2, false);

    int16_t raw_z = (int16_t)((data[1] << 8) | data[0]);
    float gyro_z = raw_z * 0.0305f / 3.2f;

    // Exponential low-pass filter
    const float alpha = 0.1f;
    ctx->gyro_z_filtered = alpha * gyro_z + (1 - alpha) * ctx->gyro_z_filtered;

    if (ctx->gyro_z_filtered < 1.0f && ctx->gyro_z_filtered > -1.0f)
        ctx->gyro_z_filtered = 0.0f;

    return ctx->gyro_z_filtered;
}


void bmi160_update_orientation(bmi160_ctx_t *ctx) {
    absolute_time_t now = get_absolute_time();
    float dt = absolute_time_diff_us(ctx->last_time, now) / 1e6f;
    ctx->last_time = now;

    float gyro_z = read_gyro_z(ctx);
    ctx->accumulated_orientation += gyro_z * dt;

    while (ctx->accumulated_orientation >= 360.0f) ctx->accumulated_orientation -= 360.0f;
    while (ctx->accumulated_orientation < 0.0f)    ctx->accumulated_orientation += 360.0f;
}

int bmi160_get_orientation(const bmi160_ctx_t *ctx) {
    float angle = fmodf(ctx->accumulated_orientation, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    
    // Sector centering: add 22.5 before dividing
    int sector = ((int)((angle + 22.5f) / 45.0f)) % 8 + 1;

    return sector;
}
