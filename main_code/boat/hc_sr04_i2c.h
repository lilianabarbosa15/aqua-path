#ifndef HC_SR04_I2C_H
#define HC_SR04_I2C_H

#include <stdint.h>

#define HC_SR04_I2C_ADDR 0x57   // Dirección I2C por defecto
#define I2C_PORT i2c0           // Cambiar i2c1 si lo necesitas
#define SDA_PIN 4               // Pines por defecto
#define SCL_PIN 5

// Inicializa el bus I2C para el sensor HC-SR04 I2C
void hc_sr04_i2c_init(void);

// Solicita una medición y devuelve la distancia en centímetros
uint32_t hc_sr04_ping_cm(void);

#endif
