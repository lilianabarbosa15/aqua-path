#ifndef QMC5883L_H
#define QMC5883L_H

#include <stdbool.h>

// Direction and pins
#define QMC5883L_ADDR  0x0D

// Parameters
#define DECLINATION    -7.78f

// Offsets (after calibrate)
extern float offsetX;
extern float offsetY;

typedef struct {
    i2c_inst_t *i2c;
    int sda_pin;
    int scl_pin;
} qmc5883l_ctx_t;

// Inicializa el sensor
void qmc5883l_init(qmc5883l_ctx_t *ctx);

// Lee datos crudos del sensor
bool qmc5883l_read_raw(qmc5883l_ctx_t *ctx, int16_t *x, int16_t *y, int16_t *z);

// Convierte heading en grados a texto cardinal
int deg_name(float headingDeg);

#endif
