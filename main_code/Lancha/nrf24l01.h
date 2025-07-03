#ifndef NRF24L01_H
#define NRF24L01_H

#include "hardware/spi.h"
#include "pico/stdlib.h"

// Pines SPI1 de Raspberry Pi Pico (puedes cambiarlos)
#define NRF_CE   15
#define NRF_CSN  14
#define SPI_PORT spi1

// === Funciones comunes ===
void nrf24_init();
void nrf24_write_byte(uint8_t reg, uint8_t value);
void nrf24_write_register(uint8_t reg, uint8_t *data, uint8_t len);

// === Transmisor ===
void nrf24_config_tx(uint8_t *tx_addr);
void nrf24_send(uint8_t *data, uint8_t len);
bool nrf24_is_sending();

// === Receptor ===
void nrf24_config_rx(uint8_t *rx_addr, uint8_t payload_size);
bool nrf24_data_ready();
void nrf24_receive(uint8_t *buffer, uint8_t len);
uint8_t nrf24_read_status();


#endif
