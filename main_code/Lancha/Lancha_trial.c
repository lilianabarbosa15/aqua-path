#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT        i2c0
#define SDA_PIN         4
#define SCL_PIN         5
#define BMI160_ADDR     0x68
#define ACCEL_SENS      16384.0f   // ±2 g  -> LSB/g
#define GYRO_SENS       131.0f     // ±250 dps -> LSB/(°/s)

// --- UMBRALES DE MOVIMIENTO ---------------------------------------------
#define ACCEL_THRES     1.00f      // [m/s²]  des-vío respecto a 1 g
#define GYRO_THRES      4.00f      // [°/s]   norma angular
// ------------------------------------------------------------------------

void bmi160_write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, BMI160_ADDR, buf, 2, false);
}
void bmi160_read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(I2C_PORT, BMI160_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, BMI160_ADDR, buf, len, false);
}
static inline int16_t to_i16(uint8_t lsb, uint8_t msb) {
    return (int16_t)((msb << 8) | lsb);
}
void read_accel(float *ax, float *ay, float *az) {
    uint8_t d[6];  bmi160_read_regs(0x12, d, 6);
    *ax = to_i16(d[0], d[1]) * (9.81f / ACCEL_SENS);
    *ay = to_i16(d[2], d[3]) * (9.81f / ACCEL_SENS);
    *az = to_i16(d[4], d[5]) * (9.81f / ACCEL_SENS);
}
void read_gyro(float *gx, float *gy, float *gz) {
    uint8_t d[6];  bmi160_read_regs(0x0C, d, 6);
    *gx = to_i16(d[0], d[1]) / GYRO_SENS;
    *gy = to_i16(d[2], d[3]) / GYRO_SENS;
    *gz = to_i16(d[4], d[5]) / GYRO_SENS;
}

int main() {
    stdio_init_all();

    // I2C
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);  gpio_pull_up(SCL_PIN);

    // LED a bordo (GP25)
    const uint LED_PIN = 25;
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);

    sleep_ms(500);
    bmi160_write_reg(0x7E, 0x11);      // acel. modo normal
    sleep_ms(50);
    bmi160_write_reg(0x7E, 0x15);      // gyro modo normal
    sleep_ms(100);
    bmi160_write_reg(0x7E, 0x37);      // auto-calibración acel
    sleep_ms(1000);

    float ax, ay, az, gx, gy, gz;

    while (true) {
        read_accel(&ax, &ay, &az);
        read_gyro (&gx, &gy, &gz);

        // Magnitudes
        float gyro_norm  = sqrtf(gx*gx + gy*gy + gz*gz);
        float accel_norm = sqrtf(ax*ax + ay*ay + az*az);
        float accel_dev  = fabsf(accel_norm - 9.81f);

        bool moving = (gyro_norm  > GYRO_THRES) ||
                      (accel_dev  > ACCEL_THRES);

        gpio_put(LED_PIN, moving); // LED ON cuando se mueve

        printf("AX=%.2f AY=%.2f AZ=%.2f "
               "GX=%.2f GY=%.2f GZ=%.2f "
               "STATE=%s\n",
               ax, ay, az, gx, gy, gz,
               moving ? "MOV" : "STAB");

        sleep_ms(100); // 10 Hz
    }
    return 0;
}
