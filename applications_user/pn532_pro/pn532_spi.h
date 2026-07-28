#pragma once

#include <furi.h>
#include <furi_hal.h>

#define PN532_SPI_STATREAD  0x02
#define PN532_SPI_DATAWRITE 0x01
#define PN532_SPI_DATAREAD  0x03
#define PN532_SPI_READY     0x01

typedef struct {
    const GpioPin* cs_pin;
    const GpioPin* rst_pin;
} Pn532Spi;

Pn532Spi* pn532_spi_alloc(void);
void pn532_spi_free(Pn532Spi* instance);
void pn532_spi_init(Pn532Spi* instance);
void pn532_spi_reset(Pn532Spi* instance);
bool pn532_spi_wait_ready(Pn532Spi* instance, uint32_t timeout_ms);
bool pn532_spi_write_frame(Pn532Spi* instance, const uint8_t* data, uint16_t len);
bool pn532_spi_read_frame(Pn532Spi* instance, uint8_t* buffer, uint16_t len, uint32_t timeout_ms);
