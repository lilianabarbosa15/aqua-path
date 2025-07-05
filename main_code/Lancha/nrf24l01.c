#include "nrf24l01.h"

#define NRF_WRITE_REG     0x20
#define NRF_FLUSH_TX      0xE1
#define NRF_W_TX_PAYLOAD  0xA0
#define NRF_STATUS        0x07
#define NRF_CONFIG        0x00
#define NRF_TX_ADDR       0x10
#define NRF_RF_CH         0x05
#define NRF_SETUP_RETR    0x04
#define NRF_EN_AA         0x01
#define NRF_EN_RXADDR     0x02
#define NRF_SETUP_AW      0x03
#define NRF_RF_SETUP      0x06

#define NRF_RX_ADDR_P0 0x0A
#define NRF_RX_PW_P0   0x11
#define NRF_R_RX_PAYLOAD 0x61


static void csn_low()  { gpio_put(NRF_CSN, 0); }
static void csn_high() { gpio_put(NRF_CSN, 1); }
static void ce_low()   { gpio_put(NRF_CE, 0); }
static void ce_high()  { gpio_put(NRF_CE, 1); }

void nrf24_write_register(uint8_t reg, uint8_t *data, uint8_t len) {
    csn_low();
    uint8_t cmd = NRF_WRITE_REG | reg;
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_write_blocking(SPI_PORT, data, len);
    csn_high();
}

void nrf24_write_byte(uint8_t reg, uint8_t value) {
    nrf24_write_register(reg, &value, 1);
}

void nrf24_init() {
    gpio_init(NRF_CE);
    gpio_init(NRF_CSN);
    gpio_set_dir(NRF_CE, 1);
    gpio_set_dir(NRF_CSN, 1);

    ce_low();
    csn_high();

    spi_init(SPI_PORT, 1 * 1000 * 1000); // 1 MHz
    gpio_set_function(10, GPIO_FUNC_SPI); // SCK
    gpio_set_function(11, GPIO_FUNC_SPI); // MOSI
    gpio_set_function(12, GPIO_FUNC_SPI); // MISO


    nrf24_write_byte(NRF_CONFIG, 0x0E);       // PWR_UP, EN_CRC
    nrf24_write_byte(NRF_SETUP_AW, 0x03);     // Address width = 5 bytes
    nrf24_write_byte(NRF_RF_CH, 76);          // Channel 76
    nrf24_write_byte(NRF_SETUP_RETR, 0x3F);   // Retry delay, count
    nrf24_write_byte(NRF_RF_SETUP, 0x26);     // 1 Mbps, 0 dBm
    nrf24_write_byte(NRF_EN_AA, 0x01);        // Auto ACK on pipe 0
    nrf24_write_byte(NRF_EN_RXADDR, 0x01);    // Enable data pipe 0
}

void nrf24_config_tx(uint8_t *tx_addr) {
    nrf24_write_register(NRF_TX_ADDR, tx_addr, 5);
    nrf24_write_register(NRF_RX_ADDR_P0, tx_addr, 5);
}

void nrf24_send(uint8_t *data, uint8_t len) {
    ce_low();
    csn_low();

    uint8_t cmd = NRF_W_TX_PAYLOAD;
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_write_blocking(SPI_PORT, data, len);

    csn_high();
    ce_high();
    sleep_us(15);
    ce_low();
}

bool nrf24_is_sending() {
    uint8_t status;
    csn_low();
    spi_read_blocking(SPI_PORT, NRF_STATUS, &status, 1);
    csn_high();
    return !(status & (1 << 5)); // Check TX_DS
}

bool nrf24_data_ready() {
    uint8_t status;
    csn_low();
    uint8_t cmd = 0xFF; // Dummy byte to read STATUS
    spi_write_read_blocking(SPI_PORT, &cmd, &status, 1);
    csn_high();
    return status & (1 << 6); // RX_DR bit
}

void nrf24_config_rx(uint8_t *rx_addr, uint8_t payload_size) {
    ce_low();

    nrf24_write_register(NRF_RX_ADDR_P0, rx_addr, 5);
    nrf24_write_byte(NRF_RX_PW_P0, payload_size);

    nrf24_write_byte(NRF_CONFIG, 0x0F); // PRIM_RX, PWR_UP, CRC
    nrf24_write_byte(NRF_EN_RXADDR, 0x01); // Enable data pipe 0

    ce_high();
    sleep_ms(2); // Espera a que entre en modo RX
}

void nrf24_receive(uint8_t *buffer, uint8_t len) {
    csn_low();
    uint8_t cmd = NRF_R_RX_PAYLOAD;
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_read_blocking(SPI_PORT, 0xFF, buffer, len);
    csn_high();
}

uint8_t nrf24_read_status() {
    uint8_t status;
    csn_low();
    uint8_t cmd = 0xFF;
    spi_write_read_blocking(SPI_PORT, &cmd, &status, 1);
    csn_high();
    return status;
}