#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"

#include "nmea_parser.h"
#include "qmc5883l.h"

#define BT_UART_ID uart0
#define BT_BAUD_RATE 9600
#define BT_TX_PIN 0
#define BT_RX_PIN 1

#define GPS_UART_ID uart1
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN 8
#define GPS_RX_PIN 9

#define ESC_PWM_PIN     22
#define SERVO_PWM_PIN   28
#define SERVO_CENTRO_US 1500
#define SERVO_IZQ_US    1200
#define SERVO_DER_US    1800
#define ESC_OFF_US      1000
#define ESC_LOW_US      1100

#define BUF_SIZE 256

float calcular_rumbo(int16_t x, int16_t y) {
    float fx = x - offsetX;
    float fy = y - offsetY;
    float heading = atan2f(fy, fx);
    float deg = heading * (180.0f / M_PI);
    if (deg < 0) deg += 360.0f;
    deg -= 106.0f;
    deg += DECLINATION;
    if (deg < 0) deg += 360.0f;
    if (deg >= 360.0f) deg -= 360.0f;
    return deg;
}

void init_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 64.0f);
    pwm_config_set_wrap(&cfg, 39062);
    pwm_init(slice, &cfg, true);
}

void set_pwm_us(uint pin, uint16_t us) {
    uint slice = pwm_gpio_to_slice_num(pin);
    uint16_t level = (us * 39062) / 20000;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), level);
}

char cmd_buffer[64];
int cmd_index = 0;

int main() {
    stdio_init_all();

    uart_init(BT_UART_ID, BT_BAUD_RATE);
    gpio_set_function(BT_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(BT_RX_PIN, GPIO_FUNC_UART);

    uart_init(GPS_UART_ID, GPS_BAUD_RATE);
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);

    qmc5883l_init();
    init_pwm(ESC_PWM_PIN);
    init_pwm(SERVO_PWM_PIN);

    set_pwm_us(ESC_PWM_PIN, 1000);
    set_pwm_us(SERVO_PWM_PIN, SERVO_CENTRO_US);

    const char *inicio = "\r\n✅ Sistema listo. Enviando (lat,lon,rumbo) y recibiendo velocidad/direccion.\r\n";
    uart_write_blocking(BT_UART_ID, (const uint8_t *)inicio, strlen(inicio));

    char gps_line[BUF_SIZE];
    int index = 0;
    gps_data_t gps;
    kalman_t kalman_lat;
    kalman_t kalman_lon;
    kalman_init(&kalman_lat, 7, 1, 0.00001);
    kalman_init(&kalman_lon, -80, 1, 0.00001);

    while (true) {
        // Procesar comandos recibidos por Bluetooth (no bloqueante)
        if (uart_is_readable(BT_UART_ID)) {
            char c = uart_getc(BT_UART_ID);
            uart_putc(BT_UART_ID, c); // ECO para confirmar
            if (c == '\n' || c == '\r') {
                cmd_buffer[cmd_index] = '\0';
                if (cmd_index > 0) {
                    if (strncmp(cmd_buffer, "velocidad ", 10) == 0) {
                        int velocidad = atoi(cmd_buffer + 10);
                        if (velocidad >= 0 && velocidad <= 100) {
                            uint16_t us = ESC_OFF_US + (velocidad * (ESC_LOW_US - ESC_OFF_US) / 100);
                            set_pwm_us(ESC_PWM_PIN, us);
                        }
                    } else if (strncmp(cmd_buffer, "posicion ", 9) == 0) {
                        int posicion = atoi(cmd_buffer + 9);
                        if (posicion >= 1 && posicion <= 360) {
                            int16_t us = SERVO_CENTRO_US + (posicion - 180) * (SERVO_DER_US - SERVO_CENTRO_US) / 180;
                            set_pwm_us(SERVO_PWM_PIN, us);
                        }
                    }
                    cmd_index = 0;
                }
            } else if (cmd_index < sizeof(cmd_buffer) - 1) {
                cmd_buffer[cmd_index++] = c;
            }
        }

        // Leer datos del GPS
        if (uart_is_readable(GPS_UART_ID)) {
            char c = uart_getc(GPS_UART_ID);
            if (c == '\n' || index >= BUF_SIZE - 1) {
                gps_line[index] = '\0';
                index = 0;

                if (gps_line[0] == '$' && nmea_parse_line(gps_line, &gps)) {
                    double lat_f = kalman_update(&kalman_lat, gps.latitude);
                    double lon_f = kalman_update(&kalman_lon, gps.longitude);

                    int16_t x, y, z;
                    float heading = 0.0f;
                    if (qmc5883l_read_raw(&x, &y, &z)) {
                        heading = calcular_rumbo(x, y);
                    }

                    char mensaje[128];
                    snprintf(mensaje, sizeof(mensaje), "(%.6f,%.6f,%d)\r\n", lat_f, lon_f, (int)(heading + 0.5f));
                    uart_write_blocking(BT_UART_ID, (const uint8_t *)mensaje, strlen(mensaje));
                }
            } else if (c != '\r') {
                gps_line[index++] = c;
            }
        }
        sleep_ms(1);
    }
}
