/*
 * Aqua Path - Autonomous Boat Project
 * This code is for an autonomous boat that uses a magnetometer, GPS, and Bluetooth to navigate.
 * It reads commands from a Bluetooth device to control speed and position, and sends GPS data back.
 * The boat uses PWM for ESC (Electronic Speed Controller) and servo motor control.
 * By Miguel Angel Alvarez, Liliana Barbosa, and Kevin Echeverri
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "pico/time.h"
#include "hardware/i2c.h"

#include "nmea_parser.h"
#include "hc_sr04.h"
#include "qmc5883l.h"

// Config of pins BT device
#define BT_UART_ID uart0
#define BT_BAUD_RATE 9600
#define BT_TX_PIN 0
#define BT_RX_PIN 1

// Config of I2C for magnetometer
#define MAGNETOMETER_I2C i2c0
#define MAGNETOMETER_SDA_PIN 4
#define MAGNETOMETER_SCL_PIN 5

// Config of pins GPS device
#define GPS_UART_ID uart1
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN 8    
#define GPS_RX_PIN 9

// Config of ultrasonic sensor
#define TRIG_PIN 16
#define ECHO_PIN 17

// Config of Boat pins
#define ESC_PWM_PIN     22
#define SERVO_PWM_PIN   28
#define SERVO_CENTRO_US 1500
#define SERVO_IZQ_US    1200
#define SERVO_DER_US    1800
#define ESC_OFF_US      1000
#define ESC_LOW_US      1100

#define BUF_SIZE 256

#define DISTANCE 30

// Magnetic declination for your location in degrees
float calc_heading(int16_t x, int16_t y) {
    float fx = x - offsetX;                 // offsetX and offsetY are the offsets for the magnetometer
    float fy = y - offsetY;
    float heading = atan2f(fy, fx);         // Calculate heading in radians
    float deg = heading * (180.0f / M_PI);  // Convert to degrees
    if (deg < 0) deg += 360.0f;             // Normalize to 0-360 degrees
    deg -= 106.0f;                          // Adjust for the sensor's orientation (this value may need to be calibrated)
    deg += DECLINATION;                     // DECLINATION is the magnetic declination for your location in degrees
    if (deg < 0) deg += 360.0f;             // Ensure the result is positive
    if (deg >= 360.0f) deg -= 360.0f;       // Ensure the result is less than 360 degrees
    return deg;
}

// Config of PWM for ESC and Servo
void init_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);      // Set the pin to PWM function
    uint slice = pwm_gpio_to_slice_num(pin);    // Get the PWM slice number for the pin
    pwm_config cfg = pwm_get_default_config();  // Get the default PWM configuration
    pwm_config_set_clkdiv(&cfg, 64.0f);         // Set the clock divider to 64
    pwm_config_set_wrap(&cfg, 39062);           // Set the wrap value for 20ms period (50Hz)
    pwm_init(slice, &cfg, true);                // Initialize the PWM slice with the configuration
}

// Set the PWM level in microseconds
void set_pwm_us(uint pin, uint16_t us) {
    uint slice = pwm_gpio_to_slice_num(pin);                // Get the PWM slice number for the pin
    uint16_t level = (us * 39062) / 20000;                  // Convert microseconds to PWM level (wrap value is 39062 for 20ms period)
    pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), level); // Set the PWM channel level
}

// Convert angle in degrees to PWM value in microseconds
uint16_t angle_to_pwm(int angle) {
    return SERVO_CENTRO_US + (angle - 180) * (SERVO_DER_US - SERVO_CENTRO_US) / 180; // Convert angle to PWM value
}

// Buffer for commands received via Bluetooth
char cmd_buffer[64];
int cmd_index = 0;

// Offsets for the magnetometer
volatile int degree_desired = 1;    //1-8 (sended by the Web App)
int direction = 180;                //direction of the boat
int degree = 1;                     //1-8 (degree that marks the boat)
float distance = 0;

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
    qmc5883l_ctx_t qmc = {
        .i2c = MAGNETOMETER_I2C,
        .sda_pin = MAGNETOMETER_SDA_PIN,
        .scl_pin = MAGNETOMETER_SCL_PIN
    };
    qmc5883l_init(&qmc);

    // Initialize PWM for ESC and Servo
    init_pwm(ESC_PWM_PIN);
    init_pwm(SERVO_PWM_PIN);
    set_pwm_us(ESC_PWM_PIN, 1000);
    set_pwm_us(SERVO_PWM_PIN, SERVO_CENTRO_US);

    // Set the magnetic declination for our location (in degrees)
    const char *init = "\r\n System ready. Sending (lat,lon,dir) y receiving vel/dir.\r\n";
    uart_write_blocking(BT_UART_ID, (const uint8_t *)init, strlen(init));

    // Variables for GPS
    char gps_line[BUF_SIZE];
    int index = 0;
    gps_data_t gps;

    // Initialize sensor ultrasonic
    hc_sr04_init(TRIG_PIN,ECHO_PIN);
    bool obstacle_detected = false;

    while (true) {
        // Read commands from Bluetooth
        if (uart_is_readable(BT_UART_ID)) {
            char c = uart_getc(BT_UART_ID);
            uart_putc(BT_UART_ID, c);           // ECO for debugging
            if (c == '\n' || c == '\r') {
                cmd_buffer[cmd_index] = '\0';   // end of command
                if (cmd_index > 0) {            // process command
                    if (strncmp(cmd_buffer, "velocidad ", 10) == 0) {   // set speed command
                        int velocity = atoi(cmd_buffer + 10);           // get speed value
                        if (velocity >= 0 && velocity <= 100) {         // check speed range
                            uint16_t us = ESC_OFF_US + (velocity * (ESC_LOW_US - ESC_OFF_US) / 100);   // convert to PWM value
                            set_pwm_us(ESC_PWM_PIN, us);                // set PWM for ESC
                        }
                    } else if (strncmp(cmd_buffer, "posicion ", 9) == 0) {  // set position command
                        int position = atoi(cmd_buffer + 9);            // get position value
                        if (position >= 1 && position <= 8)             // check position range
                            degree_desired = position;                  // set desired position
                    }
                    cmd_index = 0;                          // reset command index
                }
            } else if (cmd_index < sizeof(cmd_buffer) - 1)      // add character to command buffer
                cmd_buffer[cmd_index++] = c;                    // add character to buffer
        }

        // Update servo position based on desired position
        if (distance > 0 && distance < DISTANCE) {
            if (!obstacle_detected) {
                uint16_t us = angle_a_pwm(360);
                set_pwm_us(SERVO_PWM_PIN, us);
                obstacle_detected = true;
            }
        } else {
            if (obstacle_detected)
                obstacle_detected = false;
        }
        int diff = (degree_desired - degree + 8) % 8;
        if(!obstacle_detected){
            if (diff == 0 )
                direction = 180;
            else if (diff <= 4)
                direction = 1;
            else
                direction = 360;
            uint16_t us = angle_to_pwm(direction);
            set_pwm_us(SERVO_PWM_PIN, us);
        }
        

        // Read GPS data
        if (uart_is_readable(GPS_UART_ID)) {
            char c = uart_getc(GPS_UART_ID);
            if (c == '\n' || index >= BUF_SIZE - 1) {
                gps_line[index] = '\0';
                index = 0;
                // Process the GPS line
                if (gps_line[0] == '$' && nmea_parse_line(gps_line, &gps) && gps.satellites >= 3) { // check if the line is a valid NMEA sentence
                    double lat_f = gps.latitude;
                    double lon_f = gps.longitude;

                    // Read the magnetometer and calculate heading
                    int16_t x, y, z;
                    float heading = 0.0f;
                    if (qmc5883l_read_raw(&qmc, &x, &y, &z)) {
                        heading = calc_heading(x, y);         // calculate heading from magnetometer data
                        degree = deg_name(heading + 0.5f);
                    }
                    // Send the GPS data via Bluetooth
                    char mensaje[128];

                    // Distance measurement
                    distance = hc_sr04_read_cm();

                    snprintf(mensaje, sizeof(mensaje), "(%.6f,%.6f,%d)\r\n", lat_f, lon_f, degree);
                    uart_write_blocking(BT_UART_ID, (const uint8_t *)mensaje, strlen(mensaje));
                }
            } else if (c != '\r') {     // Ignore carriage return characters
                gps_line[index++] = c;  // Add character to GPS line buffer
            }
        }
        sleep_ms(1);
    }
}
