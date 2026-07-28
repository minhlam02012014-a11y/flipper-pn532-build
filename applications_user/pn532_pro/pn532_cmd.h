#pragma once

#include "pn532_spi.h"

typedef struct {
    Pn532Spi* spi;
} Pn532Dev;

Pn532Dev* pn532_dev_alloc(void);
void pn532_dev_free(Pn532Dev* dev);
bool pn532_sam_config(Pn532Dev* dev);
bool pn532_read_passive_target(Pn532Dev* dev, uint8_t* uid, uint8_t* uid_len);
