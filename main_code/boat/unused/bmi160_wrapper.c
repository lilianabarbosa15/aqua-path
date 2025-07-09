// bmi160_wrapper.c
#include "bmi160_wrapper.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>
#include <stdio.h>

#define I2C_PORT i2c0
#define BMI160_ADDR 0x69
#define RAD_TO_DEG 57.2958f

static int sda_pin_local = 4;
static int scl_pin_local = 5;
static float orientacion_acumulada = 0.0f;
static absolute_time_t ultimo_tiempo;
static float gyro_z_filtrado = 0.0f;
static int ultima_direccion = 1;
static int cuenta_confirmacion = 0;

void bmi160_iniciar(int sda_pin, int scl_pin) {
    sda_pin_local = sda_pin;
    scl_pin_local = scl_pin;

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(sda_pin_local, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin_local, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin_local);
    gpio_pull_up(scl_pin_local);

    sleep_ms(100);

    // Set command register to normal mode for gyroscope (0x15)
    uint8_t gyro_normal_mode_cmd[2] = {0x7E, 0x15};
    i2c_write_blocking(I2C_PORT, BMI160_ADDR, gyro_normal_mode_cmd, 2, false);
    sleep_ms(100);

    // Set command register to normal mode for accelerometer (0x11)
    uint8_t acc_normal_mode_cmd[2] = {0x7E, 0x11};
    i2c_write_blocking(I2C_PORT, BMI160_ADDR, acc_normal_mode_cmd, 2, false);
    sleep_ms(100);

    // Configurar rango del giroscopio a 250 dps (opcional)
    uint8_t gyro_range[2] = {0x43, 0x03}; // 0x03 = 250 dps
    i2c_write_blocking(I2C_PORT, BMI160_ADDR, gyro_range, 2, false);

    // Configurar rango del acelerómetro (opcional)
    uint8_t acc_range[2] = {0x41, 0x03}; // 0x03 = ±2g
    i2c_write_blocking(I2C_PORT, BMI160_ADDR, acc_range, 2, false);

    ultimo_tiempo = get_absolute_time();
}

// Simula lectura del giroscopio Z
float leer_gyro_z() {
    uint8_t reg = 0x10;
    uint8_t datos[2];

    i2c_write_blocking(I2C_PORT, BMI160_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, BMI160_ADDR, datos, 2, false);

    int16_t raw_z = (int16_t)((datos[1] << 8) | datos[0]);
    float gyro_z = raw_z * 0.0305f/3.2f;

    // Filtro de paso bajo exponencial
    const float alpha = 0.1f;  // Ajusta según suavidad deseada
    gyro_z_filtrado = alpha * gyro_z + (1 - alpha) * gyro_z_filtrado;
    if (gyro_z_filtrado < 1.0 && gyro_z_filtrado > -1.0){
        gyro_z_filtrado = 0.0;
    }
    return gyro_z_filtrado;
}

void bmi160_actualizar_orientacion() {
    absolute_time_t ahora = get_absolute_time();
    float dt = absolute_time_diff_us(ultimo_tiempo, ahora) / 1e6f;
    ultimo_tiempo = ahora;

    float gyro_z = leer_gyro_z();
    orientacion_acumulada += gyro_z * dt;

    // Mantener en rango 0-360
    while (orientacion_acumulada >= 360.0f) orientacion_acumulada -= 360.0f;
    while (orientacion_acumulada < 0.0f) orientacion_acumulada += 360.0f;
}

int bmi160_get_orientacion() {
    float angulo = fmodf(orientacion_acumulada, 360.0f);
    if (angulo < 0.0f) angulo += 360.0f;
    
    // Centrado de sectores: sumamos 22.5 antes de dividir
    int sector = ((int)((angulo + 22.5f) / 45.0f)) % 8 + 1;

    return sector;
}