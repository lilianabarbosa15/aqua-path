#ifndef NRF24L01_H
#define NRF24L01_H

#include "hardware/spi.h"
#include "pico/stdlib.h"

// SPI configuration
#define SPI_PORT spi1
typedef struct {
    uint ce_pin;
    uint csn_pin;
} nrf24_ctx_t;

// ----- Common Functions -----
void nrf24_init(nrf24_ctx_t *ctx);
void nrf24_write_byte(nrf24_ctx_t *ctx, uint8_t reg, uint8_t value);
void nrf24_write_register(nrf24_ctx_t *ctx, uint8_t reg, uint8_t *data, uint8_t len);

// ------- TX Functions -------
void nrf24_config_tx(nrf24_ctx_t *ctx, uint8_t *tx_addr);
void nrf24_send(nrf24_ctx_t *ctx, uint8_t *data, uint8_t len);
bool nrf24_is_sending(nrf24_ctx_t *ctx);

// ------- RX Functions -------
void nrf24_config_rx(nrf24_ctx_t *ctx, uint8_t *rx_addr, uint8_t payload_size);
bool nrf24_data_ready(nrf24_ctx_t *ctx);
void nrf24_receive(nrf24_ctx_t *ctx, uint8_t *buffer, uint8_t len);
uint8_t nrf24_read_status(nrf24_ctx_t *ctx);


#endif
