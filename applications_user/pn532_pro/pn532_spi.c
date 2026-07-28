#include "pn532_spi.h"
#include <string.h>

static uint8_t pn532_spi_swap_bits(uint8_t n) {
    n = ((n >> 1) & 0x55) | ((n << 1) & 0xAA);
    n = ((n >> 2) & 0x33) | ((n << 2) & 0xCC);
    n = ((n >> 4) & 0x0F) | ((n << 4) & 0xF0);
    return n;
}

Pn532Spi* pn532_spi_alloc(void) {
    Pn532Spi* instance = malloc(sizeof(Pn532Spi));
    instance->cs_pin = &gpio_ext_pa4;
    instance->rst_pin = &gpio_ext_pb2;
    return instance;
}

void pn532_spi_free(Pn532Spi* instance) {
    if(!instance) return;
    furi_hal_gpio_init(instance->cs_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(instance->rst_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    free(instance);
}

void pn532_spi_init(Pn532Spi* instance) {
    furi_assert(instance);
    furi_hal_gpio_init(instance->cs_pin, GpioModeOutputPushPull, GpioPullUp, GpioSpeedHigh);
    furi_hal_gpio_init(instance->rst_pin, GpioModeOutputPushPull, GpioPullUp, GpioSpeedHigh);
    furi_hal_gpio_write(instance->cs_pin, true);
    pn532_spi_reset(instance);
}

void pn532_spi_reset(Pn532Spi* instance) {
    furi_assert(instance);
    furi_hal_gpio_write(instance->rst_pin, false);
    furi_delay_ms(50);
    furi_hal_gpio_write(instance->rst_pin, true);
    furi_delay_ms(100);
}

bool pn532_spi_wait_ready(Pn532Spi* instance, uint32_t timeout_ms) {
    uint32_t start = furi_get_tick();
    uint8_t stat_cmd = pn532_spi_swap_bits(PN532_SPI_STATREAD);
    uint8_t status = 0;

    while((furi_get_tick() - start) < furi_ms_to_ticks(timeout_ms)) {
        furi_hal_spi_acquire(&furi_hal_spi_bus_handle_external);
        furi_hal_gpio_write(instance->cs_pin, false);
        furi_delay_us(10);
        
        furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_external, &stat_cmd, 1, 100);
        furi_hal_spi_bus_rx(&furi_hal_spi_bus_handle_external, &status, 1, 100);
        
        furi_delay_us(10);
        furi_hal_gpio_write(instance->cs_pin, true);
        furi_hal_spi_release(&furi_hal_spi_bus_handle_external);

        if(pn532_spi_swap_bits(status) == PN532_SPI_READY) return true;
        furi_delay_ms(5);
    }
    return false;
}

bool pn532_spi_write_frame(Pn532Spi* instance, const uint8_t* data, uint16_t len) {
    furi_assert(instance);
    
    uint8_t frame_len = len + 1;
    uint8_t write_cmd = pn532_spi_swap_bits(PN532_SPI_DATAWRITE);
    
    uint8_t header[5] = {
        0x00, 0x00, 0xFF, frame_len, (uint8_t)(~frame_len + 1)
    };

    uint8_t tx_buf[64];
    uint16_t idx = 0;
    tx_buf[idx++] = write_cmd;

    for(int i = 0; i < 5; i++) {
        tx_buf[idx++] = pn532_spi_swap_bits(header[i]);
    }

    uint8_t tfi = 0xD4;
    uint8_t sum = tfi;
    tx_buf[idx++] = pn532_spi_swap_bits(tfi);

    for(uint16_t i = 0; i < len; i++) {
        sum += data[i];
        tx_buf[idx++] = pn532_spi_swap_bits(data[i]);
    }

    tx_buf[idx++] = pn532_spi_swap_bits((uint8_t)(~sum + 1));
    tx_buf[idx++] = 0x00;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_external);
    furi_hal_gpio_write(instance->cs_pin, false);
    furi_delay_us(10);

    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_external, tx_buf, idx, 100);

    furi_delay_us(10);
    furi_hal_gpio_write(instance->cs_pin, true);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_external);
    return true;
}

bool pn532_spi_read_frame(Pn532Spi* instance, uint8_t* buffer, uint16_t len, uint32_t timeout_ms) {
    if(!pn532_spi_wait_ready(instance, timeout_ms)) return false;

    uint8_t read_cmd = pn532_spi_swap_bits(PN532_SPI_DATAREAD);
    uint8_t raw_rx[64] = {0};
    uint16_t fetch_len = (len + 10 > 64) ? 64 : (len + 10);

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_external);
    furi_hal_gpio_write(instance->cs_pin, false);
    furi_delay_us(10);

    furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_external, &read_cmd, 1, 100);
    furi_hal_spi_bus_rx(&furi_hal_spi_bus_handle_external, raw_rx, fetch_len, 500);

    furi_delay_us(10);
    furi_hal_gpio_write(instance->cs_pin, true);
    furi_hal_spi_release(&furi_hal_spi_bus_handle_external);

    for(uint16_t i = 0; i < fetch_len; i++) {
        raw_rx[i] = pn532_spi_swap_bits(raw_rx[i]);
    }

    int frame_start = -1;
    for(uint16_t i = 0; i < fetch_len - 3; i++) {
        if(raw_rx[i] == 0x00 && raw_rx[i+1] == 0xFF) {
            frame_start = i - 1;
            if(frame_start < 0) frame_start = 0;
            break;
        }
    }

    if(frame_start != -1) {
        memcpy(buffer, &raw_rx[frame_start], len);
        return true;
    }

    return false;
}
