#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "qmc5883l.h"

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17

#define DECLINACION_MAGNETICA  -4.0  // declinación en grados (ajustar según ubicación)
#define FILTRO_N 10                 // tamaño del filtro de media móvil

float filtro[FILTRO_N];
int filtro_index = 0;
bool filtro_lleno = false;

// Agrega nueva lectura al filtro de media móvil
float media_movil(float nuevo_valor) {
    filtro[filtro_index] = nuevo_valor;
    filtro_index = (filtro_index + 1) % FILTRO_N;

    int n = filtro_lleno ? FILTRO_N : filtro_index;

    float suma = 0.0;
    for (int i = 0; i < n; i++) {
        suma += filtro[i];
    }

    if (filtro_index == 0) filtro_lleno = true;

    return suma / n;
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    printf("Iniciando QMC5883L...\n");
    qmc5883l_init(I2C_PORT);

    int16_t x, y, z;

    while (true) {
        if (qmc5883l_read_raw(I2C_PORT, &x, &y, &z)) {
            // Cálculo del heading (norte magnético)
            float heading = atan2((float)y, (float)x) * (180.0 / M_PI);
            if (heading < 0) heading += 360.0;

            // Corrección por declinación magnética
            heading += DECLINACION_MAGNETICA;
            if (heading < 0) heading += 360.0;
            if (heading >= 360) heading -= 360.0;

            // Filtrar el heading con media móvil
            float heading_filtrado = media_movil(heading);

            // Imprimir resultados
            printf("X:%6d  Y:%6d  Z:%6d  | Heading: %.2f°  | Filtrado: %.2f°\n",
                   x, y, z, heading, heading_filtrado);
        } else {
            printf("Error leyendo QMC5883L\n");
        }

        sleep_ms(300);
    }
}
