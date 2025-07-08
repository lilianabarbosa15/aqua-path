#include "hc_sr04_i2c.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>

void hc_sr04_i2c_init(void) {
    i2c_init(I2C_PORT, 100 * 1000);  // 100 kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

uint32_t hc_sr04_ping_cm(void) {
    uint8_t cmd = 1;
    uint8_t ds[3];

    // Enviar comando de medición
    int res = i2c_write_blocking(I2C_PORT, HC_SR04_I2C_ADDR, &cmd, 1, false);
    if (res < 0) return 0;

    sleep_ms(120);  // Esperar tiempo de medición

    // Leer los 3 bytes de datos
    res = i2c_read_blocking(I2C_PORT, HC_SR04_I2C_ADDR, ds, 3, false);
    if (res < 3) return 0;

    // Combinar bytes y convertir a centímetros
    uint32_t raw = ((uint32_t)ds[0] << 16) | ((uint32_t)ds[1] << 8) | ds[2];
    uint32_t cm = raw / 10000;

    return (cm >= 1 && cm <= 900) ? cm : 0;
}
