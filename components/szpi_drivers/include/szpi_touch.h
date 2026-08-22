#pragma once

#include "esp_lcd_touch.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_lcd_touch_handle_t handle;
    bool touched;
    uint16_t x;
    uint16_t y;
} szpi_touch_t;

esp_err_t szpi_touch_init(szpi_touch_t *touch);
esp_err_t szpi_touch_poll(szpi_touch_t *touch);

#ifdef __cplusplus
}
#endif
