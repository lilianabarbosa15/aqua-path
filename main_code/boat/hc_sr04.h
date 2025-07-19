#ifndef HC_SR04_H
#define HC_SR04_H

void hc_sr04_init(int trig_pin, int echo_pin);
float hc_sr04_read_cm();

#endif
