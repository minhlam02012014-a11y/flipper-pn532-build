#include "pn532_cmd.h"
#include <string.h>

Pn532Dev* pn532_dev_alloc(void) {
    Pn532Dev* dev = malloc(sizeof(Pn532Dev));
    dev->spi = pn532_spi_alloc();
    pn532_spi_init(dev->spi);
    return dev;
}

void pn532_dev_free(Pn532Dev* dev) {
    if(!dev) return;
    pn532_spi_free(dev->spi);
    free(dev);
}

bool pn532_sam_config(Pn532Dev* dev) {
    uint8_t sam_cmd[3] = {0x14, 0x01, 0x14};
    return pn532_spi_write_frame(dev->spi, sam_cmd, 3);
}

bool pn532_read_passive_target(Pn532Dev* dev, uint8_t* uid, uint8_t* uid_len) {
    uint8_t scan_cmd[3] = {0x4A, 0x01, 0x00};
    if(!pn532_spi_write_frame(dev->spi, scan_cmd, 3)) return false;

    uint8_t rx_buf[32] = {0};
    if(pn532_spi_read_frame(dev->spi, rx_buf, 32, 600)) {
        int res_offset = -1;
        for(int i = 0; i < 20; i++) {
            if(rx_buf[i] == 0xD5 && rx_buf[i+1] == 0x4B) {
                res_offset = i;
                break;
            }
        }

        if(res_offset != -1 && rx_buf[res_offset + 2] > 0) {
            uint8_t len = rx_buf[res_offset + 6];
            if(len > 10) len = 10;
            *uid_len = len;

            for(uint8_t i = 0; i < len; i++) {
                uid[i] = rx_buf[res_offset + 7 + i];
            }
            return true;
        }
    }
    return false;
}
