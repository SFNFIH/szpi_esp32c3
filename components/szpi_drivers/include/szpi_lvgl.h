#pragma once

#include "esp_err.h"
#include "esp_lvgl_port.h"
#include "szpi_lcd.h"
#include "szpi_touch.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_display_t *disp;
    lv_indev_t *touch_indev;
    bool disp_ready;
    bool touch_ready;
} szpi_lvgl_t;

esp_err_t szpi_lvgl_init(szpi_lvgl_t *lvgl, const szpi_lcd_t *lcd, const szpi_touch_t *touch);

#ifdef __cplusplus
}
#endif
