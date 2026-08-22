#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GXHTC3_I2C_ADDR_DEFAULT 0x70

typedef struct {
    i2c_master_dev_handle_t dev;
} gxhtc3_handle_t;

esp_err_t gxhtc3_init(gxhtc3_handle_t *handle, i2c_master_bus_handle_t bus, uint8_t addr);
esp_err_t gxhtc3_read(gxhtc3_handle_t *handle, float *temperature_c, float *humidity_rh);

#ifdef __cplusplus
}
#endif
