// bmi160_wrapper.h
#ifndef BMI160_WRAPPER_H
#define BMI160_WRAPPER_H

#include <stdint.h>

// Inicializa el BMI160 con los pines de I2C dados
void bmi160_iniciar(int sda_pin, int scl_pin);

// Lee el sensor y actualiza la orientación estimada
void bmi160_actualizar_orientacion();

// Devuelve la dirección como un dígito del 1 al 8
int bmi160_get_orientacion();

#endif // BMI160_WRAPPER_H