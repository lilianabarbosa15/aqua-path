/**
 * HC-SR04 Ultrasonic Sensor Driver
 *
 * This driver provides an interface to the HC-SR04 ultrasonic distance sensor.
 * It allows for initialization of the sensor and reading distance measurements.
 */
#ifndef HC_SR04_H
#define HC_SR04_H

void hc_sr04_init(int trig_pin, int echo_pin);
float hc_sr04_read_cm();

#endif
