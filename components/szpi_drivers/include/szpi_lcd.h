#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_handle_t panel;
} szpi_lcd_t;

esp_err_t szpi_lcd_init(szpi_lcd_t *lcd);
esp_err_t szpi_lcd_fill_color(szpi_lcd_t *lcd, uint16_t color565);
esp_err_t szpi_lcd_set_backlight(bool on);

#ifdef __cplusplus
}
#endif
