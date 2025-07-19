// bmi160_wrapper.h
#ifndef BMI160_WRAPPER_H
#define BMI160_WRAPPER_H

#include "hardware/i2c.h"
#include "pico/time.h"

typedef struct {
    i2c_inst_t *i2c;
    uint8_t address;
    int sda_pin;
    int scl_pin;
    float accumulated_orientation;
    float gyro_z_filtered;
    absolute_time_t last_time;
} bmi160_ctx_t;

// Initializes the BMI160 using the provided pins and I2C instance
void bmi160_init(bmi160_ctx_t *ctx);

// Updates the estimated orientation from gyroscope data
void bmi160_update_orientation(bmi160_ctx_t *ctx);

// Returns the orientation as a sector number from 1 to 8
int bmi160_get_orientation(const bmi160_ctx_t *ctx);


#endif // BMI160_WRAPPER_H
