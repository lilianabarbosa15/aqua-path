/*
 * Aqua Path - Autonomous Boat Project
 * This code is for an autonomous boat that uses a magnetometer, GPS, and Bluetooth to navigate.
 * It reads commands from a Bluetooth device to control speed and position, and sends GPS data back.
 * The boat uses PWM for ESC (Electronic Speed Controller) and servo motor control.
 * By Miguel Angel Alvarez, Liliana Barbosa, and Kevin
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "pico/time.h"
#include "hc_sr04_i2c.h"
#include "nmea_parser.h"
#include "qmc5883l.h"

// Config of pins BT device
#define BT_UART_ID uart0
#define BT_BAUD_RATE 9600
#define BT_TX_PIN 0
#define BT_RX_PIN 1

// Config of pins GPS device
#define GPS_UART_ID uart1
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN 8
#define GPS_RX_PIN 9

// Config of Boat pins
#define ESC_PWM_PIN     22
#define SERVO_PWM_PIN   28
#define SERVO_CENTRO_US 1500
#define SERVO_IZQ_US    1200
#define SERVO_DER_US    1800
#define ESC_OFF_US      1000
#define ESC_LOW_US      1100

#define BUF_SIZE 256

// Magnetic declination for your location in degrees
float calcular_rumbo(int16_t x, int16_t y) {
    float fx = x - offsetX; // offsetX and offsetY are the offsets for the magnetometer
    float fy = y - offsetY;
    float heading = atan2f(fy, fx); // Calculate heading in radians
    float deg = heading * (180.0f / M_PI); // Convert to degrees
    if (deg < 0) deg += 360.0f; // Normalize to 0-360 degrees
    deg -= 106.0f; // Adjust for the sensor's orientation (this value may need to be calibrated)
    deg += DECLINATION; // DECLINATION is the magnetic declination for your location in degrees
    if (deg < 0) deg += 360.0f; // Ensure the result is positive
    if (deg >= 360.0f) deg -= 360.0f; // Ensure the result is less than 360 degrees
    return deg;
}

// Config of PWM for ESC and Servo
void init_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM); // Set the pin to PWM function
    uint slice = pwm_gpio_to_slice_num(pin); // Get the PWM slice number for the pin
    pwm_config cfg = pwm_get_default_config(); // Get the default PWM configuration
    pwm_config_set_clkdiv(&cfg, 64.0f); // Set the clock divider to 64
    pwm_config_set_wrap(&cfg, 39062); // Set the wrap value for 20ms period (50Hz)
    pwm_init(slice, &cfg, true); // Initialize the PWM slice with the configuration
}

// Set the PWM level in microseconds
void set_pwm_us(uint pin, uint16_t us) {
    uint slice = pwm_gpio_to_slice_num(pin); // Get the PWM slice number for the pin
    uint16_t level = (us * 39062) / 20000; // Convert microseconds to PWM level (wrap value is 39062 for 20ms period)
    pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), level); // Set the PWM channel level
}

// Convert angle in degrees to PWM value in microseconds
uint16_t angulo_a_pwm(int angulo) {
    return SERVO_CENTRO_US + (angulo - 180) * (SERVO_DER_US - SERVO_CENTRO_US) / 180; // Convert angle to PWM value
}

// Buffer for commands received via Bluetooth
char cmd_buffer[64];
int cmd_index = 0;

// Offsets for the magnetometer (these values should be calibrated for your specific sensor)
volatile int posicion_deseada = 180;
int posicion_actual = 180;
absolute_time_t ultima_actualizacion;



int main() {
    stdio_init_all(); // Initialize standard I/O

    // Initialize UART for Bluetooth
    uart_init(BT_UART_ID, BT_BAUD_RATE);
    gpio_set_function(BT_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(BT_RX_PIN, GPIO_FUNC_UART);

    // Initialize UART for GPS
    uart_init(GPS_UART_ID, GPS_BAUD_RATE);
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);

    // Set up I2C for the magnetometer
    qmc5883l_init();
    init_pwm(ESC_PWM_PIN);
    init_pwm(SERVO_PWM_PIN);

    // Set the PWM frequency for ESC and Servo
    set_pwm_us(ESC_PWM_PIN, 1000);
    set_pwm_us(SERVO_PWM_PIN, SERVO_CENTRO_US);

    // Initialize the magnetometer offsets (these should be calibrated)
    ultima_actualizacion = get_absolute_time();

    // Set the magnetic declination for your location (in degrees)
    const char *inicio = "\r\n System ready. Sending (lat,lon,dir) y receiving vel/dir.\r\n";
    uart_write_blocking(BT_UART_ID, (const uint8_t *)inicio, strlen(inicio));

    // Variables for GPS data and Kalman filter
    char gps_line[BUF_SIZE];
    int index = 0;
    gps_data_t gps;
    kalman_t kalman_lat;
    kalman_t kalman_lon;
    kalman_init(&kalman_lat, 7, 1, 0.00001);
    kalman_init(&kalman_lon, -80, 1, 0.00001);
    bool obstaculo_detectado = false;
    while (true) {
        // Leer comandos desde Bluetooth
        if (uart_is_readable(BT_UART_ID)) {
            char c = uart_getc(BT_UART_ID);
            uart_putc(BT_UART_ID, c); // ECO para depuración
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
                            posicion_deseada = posicion;
                        }
                    }
                    cmd_index = 0;
                }
            } else if (cmd_index < sizeof(cmd_buffer) - 1) {
                cmd_buffer[cmd_index++] = c;
            }
        }

        // Verificar distancia con el sensor ultrasónico
        uint32_t distancia = hc_sr04_ping_cm();
        if (distancia > 0 && distancia < 30) {
            if (!obstaculo_detectado) {
                printf("🚧 Obstáculo detectado a %lu cm. Girando a la izquierda.\n", distancia);
                set_pwm_us(SERVO_PWM_PIN, SERVO_IZQ_US);
                obstaculo_detectado = true;
            }
        } else {
            if (obstaculo_detectado) {
                printf("✅ Camino despejado. Retomando control automático.\n");
                obstaculo_detectado = false;
            }
        }

        // Actualizar posición del servo si no hay obstáculo
        if (!obstaculo_detectado &&
            absolute_time_diff_us(ultima_actualizacion, get_absolute_time()) > 50000) {
            ultima_actualizacion = get_absolute_time();
            if (posicion_actual < posicion_deseada) {
                posicion_actual++;
            } else if (posicion_actual > posicion_deseada) {
                posicion_actual--;
            }
            uint16_t us = angulo_a_pwm(posicion_actual);
            set_pwm_us(SERVO_PWM_PIN, us);
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
