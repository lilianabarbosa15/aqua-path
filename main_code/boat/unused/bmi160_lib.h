#ifndef _BMI160_PICO_H_
#define _BMI160_PICO_H_

#include "hardware/i2c.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Conversión por defecto
 *  ±2 g  →  16384 LSB/g
 *  ±2000 °/s  →  16.4 LSB/(°/s)
 */
#define BMI160_ACC_LSB_PER_G   16384.0f
#define BMI160_GYR_LSB_PER_DPS   16.4f

typedef struct
{
    float ax_g;   ///< Aceleración X en g
    float ay_g;   ///< Aceleración Y en g
    float az_g;   ///< Aceleración Z en g
    float gx_dps; ///< Velocidad angular X en °/s
    float gy_dps; ///< Velocidad angular Y en °/s
    float gz_dps; ///< Velocidad angular Z en °/s
} bmi160_data_t;

/**
 * @brief Inicializa el BMI160 en modo normal (acel ±2 g, gyro ±2000 °/s)
 * @param i2c_inst    Instancia I²C (ej. i2c0)
 * @param sda_pin     GPIO SDA
 * @param scl_pin     GPIO SCL
 * @param address     Dirección I²C (0x68 si CSB-GND, 0x69 si CSB-VCC)
 * @return true si el dispositivo respondió al WHO_AM_I.
 */
bool bmi160_init(i2c_inst_t *i2c_inst,
                 uint sda_pin, uint scl_pin,
                 uint8_t address);

/**
 * @brief Lee acelerómetro y giróscopo, ya convertidos a unidades físicas
 * @param i2c_inst Instancia I²C usada en init
 * @param address  Dirección I²C usada en init
 * @param out      Estructura donde se guardan los datos convertidos
 * @return true si la lectura fue exitosa
 */
bool bmi160_read(i2c_inst_t *i2c_inst,
                 uint8_t address,
                 bmi160_data_t *out);

#ifdef __cplusplus
}
#endif
#endif /* _BMI160_PICO_H_ */
