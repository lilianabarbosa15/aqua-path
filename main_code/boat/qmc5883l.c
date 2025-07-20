/*
 * Aqua Path - Autonomous Boat Project
 * This code is for an autonomous boat that uses a magnetometer, GPS, and Bluetooth to navigate.
 * It reads commands from a Bluetooth device to control speed and position, and sends GPS data back.
 */
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "qmc5883l.h"
#include <math.h>

// Pins and Registers
#define REG_DATA    0x00
#define REG_STATUS  0x06
#define REG_CONTROL 0x09
#define REG_RESET   0x0B

float offsetX = 0;
float offsetY = 0;

void qmc5883l_init(qmc5883l_ctx_t *ctx) {
    i2c_init(ctx->i2c, 100000);
    gpio_set_function(ctx->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(ctx->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(ctx->sda_pin);
    gpio_pull_up(ctx->scl_pin);

    sleep_ms(100);

    uint8_t rst[2] = { REG_RESET, 0x01 };
    i2c_write_blocking(ctx->i2c, QMC5883L_ADDR, rst, 2, false);
    sleep_ms(10);

    uint8_t cfg[2] = { REG_CONTROL, 0x1D }; // 200Hz, continuo, ±2G, 512 oversampling
    i2c_write_blocking(ctx->i2c, QMC5883L_ADDR, cfg, 2, false);
    sleep_ms(10);
}

bool qmc5883l_read_raw(qmc5883l_ctx_t *ctx, int16_t *x, int16_t *y, int16_t *z) {
    uint8_t st;
    i2c_write_blocking(ctx->i2c, QMC5883L_ADDR, (uint8_t[]){REG_STATUS}, 1, true);
    i2c_read_blocking(ctx->i2c, QMC5883L_ADDR, &st, 1, false);
    if ((st & 0x01) == 0) return false;

    uint8_t buf[6];
    i2c_write_blocking(ctx->i2c, QMC5883L_ADDR, (uint8_t[]){REG_DATA}, 1, true);
    i2c_read_blocking(ctx->i2c, QMC5883L_ADDR, buf, 6, false);

    *x = (int16_t)((buf[1] << 8) | buf[0]);
    *y = (int16_t)((buf[3] << 8) | buf[2]);
    *z = (int16_t)((buf[5] << 8) | buf[4]);
    return true;
}

int deg_name(float headingDeg) {
    if (headingDeg >= 337 || headingDeg <= 23)
        return 1;     //North
    else if (headingDeg >= 23 && headingDeg <= 68)
        return 8;     //Northwest
    else if (headingDeg >= 68 && headingDeg <= 113)
        return 7;     //West
    else if (headingDeg >= 113 && headingDeg <= 158)
        return 6;     //Southwest
    else if (headingDeg >= 158 && headingDeg <= 203)
        return 5;     //South
    else if (headingDeg >= 203 && headingDeg <= 248)
        return 4;     //Southeast
    else if (headingDeg >= 248 && headingDeg <= 293)
        return 3;     //East
    else if (headingDeg >= 293 && headingDeg <= 337)
        return 2;     //Northeast
    else
        return 1;
}
