#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "bmi160_lib.h"

#define ESC_PWM_PIN     22
#define SERVO_PWM_PIN   28
#define SERVO_CENTRO_US 1500
#define SERVO_IZQ_US    1200
#define SERVO_DER_US    1800
#define ESC_OFF_US      1000
#define ESC_LOW_US      1100

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

int calcular_diferencia(int actual, int objetivo) {
    int diff = (objetivo - actual + 8) % 8;
    return diff;
}

void orientar_a_direccion(int objetivo) {
    const char* etiquetas[] = {
        "Norte", "Noreste", "Este", "Sureste",
        "Sur", "Suroeste", "Oeste", "Noroeste"
    };

    printf("Iniciando orientación hacia %s (%d)...\n", etiquetas[objetivo - 1], objetivo);

    while (true) {
        bmi160_actualizar_orientacion();
        int actual = bmi160_get_orientacion();
        printf("🧭 Dirección actual: %d (%s)\n", actual, etiquetas[actual - 1]);

        if (actual == objetivo) {
            printf("✅ Orientación alcanzada.\n");
            break;
        }

        int diff = calcular_diferencia(actual, objetivo);
        bool girar_derecha = (diff <= 4);

        set_pwm_us(SERVO_PWM_PIN, girar_derecha ? SERVO_DER_US : SERVO_IZQ_US);
        set_pwm_us(ESC_PWM_PIN, ESC_LOW_US);
        sleep_ms(800);
        set_pwm_us(ESC_PWM_PIN, ESC_OFF_US);
        set_pwm_us(SERVO_PWM_PIN, SERVO_CENTRO_US);
        sleep_ms(500);
    }
}

void leer_linea_usb(char *buffer, size_t max_len) {
    size_t idx = 0;
    while (true) {
        int ch = getchar_timeout_us(0);
        if (ch == PICO_ERROR_TIMEOUT) {
            sleep_ms(10);
            continue;
        }
        putchar(ch);
        if (ch == '\n' || ch == '\r') {
            buffer[idx] = '\0';
            if (idx > 0) {
                printf("\nRecibido: [%s]\n", buffer);
                return;
            }
        } else if (idx < max_len - 1) {
            buffer[idx++] = (char)ch;
        }
    }
}
int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Inicializando sensor BMI160...\n");
    bmi160_iniciar(4, 5);

    init_pwm(ESC_PWM_PIN);
    init_pwm(SERVO_PWM_PIN);

    // Inicialización del ESC
    printf("Inicializando ESC...\n");
    set_pwm_us(ESC_PWM_PIN, 2000);  // Pulso alto
    sleep_ms(2000);
    set_pwm_us(ESC_PWM_PIN, 1000);  // Pulso bajo (reposo)
    sleep_ms(2000);
    printf("ESC listo.\n");

    // Centrar el servo al inicio
    set_pwm_us(SERVO_PWM_PIN, SERVO_CENTRO_US);

    char buffer[64];
    printf("Listo. Escribe: posicion <1–8>\n");

    while (true) {
        leer_linea_usb(buffer, sizeof(buffer));

        if (strncmp(buffer, "posicion ", 9) == 0) {
            int objetivo = atoi(&buffer[9]);
            if (objetivo >= 1 && objetivo <= 8) {
                orientar_a_direccion(objetivo);
            } else {
                printf("Dirección inválida. Usa un valor entre 1 y 8.\n");
            }
        } else {
            printf("Comando no válido. Usa: posicion <1–8>\n");
        }
    }
}
