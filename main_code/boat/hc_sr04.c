#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hc_sr04.h"

static int TRIG_PIN;
static int ECHO_PIN;

void hc_sr04_init(int trig_pin, int echo_pin) {
    TRIG_PIN = trig_pin;
    ECHO_PIN = echo_pin;

    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

float hc_sr04_read_cm() {
    // send a 10us pulse to TRIG_PIN
    gpio_put(TRIG_PIN, 0);
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    // Wait for ECHO to go high (safety timeout)
    absolute_time_t start = get_absolute_time();
    while (!gpio_get(ECHO_PIN)) {
        if (absolute_time_diff_us(start, get_absolute_time()) > 20000)
            return -1.0f;       // Timeout
    }

    absolute_time_t echo_start = get_absolute_time();

    // Wait for ECHO to go low
    while (gpio_get(ECHO_PIN)) {
        if (absolute_time_diff_us(echo_start, get_absolute_time()) > 30000)
            return -1.0f;       // Timeout
    }

    absolute_time_t echo_end = get_absolute_time();
    int64_t duration_us = absolute_time_diff_us(echo_start, echo_end);

    // Distance in cm: (duration_us / 2) / 29.1
    float distance_cm = (float)duration_us / 58.0f;

    return distance_cm;
}
