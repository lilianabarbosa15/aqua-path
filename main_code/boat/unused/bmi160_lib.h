#ifndef _BMI160_PICO_H_
#define _BMI160_PICO_H_

#include "hardware/i2c.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default conversion factors
 *  ±2 g     →  16384 LSB/g
 *  ±2000 °/s →  16.4 LSB/(°/s)
 */
#define BMI160_ACC_LSB_PER_G     16384.0f
#define BMI160_GYR_LSB_PER_DPS   16.4f

typedef struct
{
    float ax_g;   ///< Acceleration X in g
    float ay_g;   ///< Acceleration Y in g
    float az_g;   ///< Acceleration Z in g
    float gx_dps; ///< Angular velocity X in °/s
    float gy_dps; ///< Angular velocity Y in °/s
    float gz_dps; ///< Angular velocity Z in °/s
} bmi160_data_t;

/**
 * @brief Initializes the BMI160 in normal mode (acc ±2 g, gyro ±2000 °/s)
 * @param i2c_inst    I2C instance (e.g., i2c0)
 * @param sda_pin     SDA GPIO
 * @param scl_pin     SCL GPIO
 * @param address     I2C address (0x68 if CSB-GND, 0x69 if CSB-VCC)
 * @return true if the device responded to WHO_AM_I.
 */
bool bmi160_init(i2c_inst_t *i2c_inst,
                 uint sda_pin, uint scl_pin,
                 uint8_t address);

/**
 * @brief Reads accelerometer and gyroscope, already converted to physical units
 * @param i2c_inst I2C instance used in init
 * @param address  I2C address used in init
 * @param out      Structure to store the converted data
 * @return true if the read was successful
 */
bool bmi160_read(i2c_inst_t *i2c_inst,
                 uint8_t address,
                 bmi160_data_t *out);

#ifdef __cplusplus
}
#endif
#endif /* _BMI160_PICO_H_ */
