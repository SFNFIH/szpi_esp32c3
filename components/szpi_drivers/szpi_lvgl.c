#include "szpi_lvgl.h"

#include <string.h>
#include "board_pins.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "szpi_lvgl";

esp_err_t szpi_lvgl_init(szpi_lvgl_t *lvgl, const szpi_lcd_t *lcd, const szpi_touch_t *touch)
{
    ESP_RETURN_ON_FALSE(lvgl != NULL && lcd != NULL, ESP_ERR_INVALID_ARG, TAG, "invalid arg");
    memset(lvgl, 0, sizeof(*lvgl));

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "lvgl port init failed");

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd->io,
        .panel_handle = lcd->panel,
        .buffer_size = BOARD_LCD_H_RES * 40,
        .double_buffer = true,
        .hres = BOARD_LCD_H_RES,
        .vres = BOARD_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = true,
            .mirror_x = true,
            .mirror_y = false,
        },
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .flags = {
            .buff_dma = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = false,
#endif
        },
    };

    lvgl->disp = lvgl_port_add_disp(&disp_cfg);
    ESP_RETURN_ON_FALSE(lvgl->disp != NULL, ESP_FAIL, TAG, "add display failed");
    lvgl->disp_ready = true;

    if (touch != NULL && touch->handle != NULL) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lvgl->disp,
            .handle = touch->handle,
        };
        lvgl->touch_indev = lvgl_port_add_touch(&touch_cfg);
        if (lvgl->touch_indev != NULL) {
            lvgl->touch_ready = true;
        } else {
            ESP_LOGW(TAG, "touch input not added");
        }
    }

    ESP_LOGI(TAG, "LVGL ready (%dx%d, disp=%d touch=%d)",
             BOARD_LCD_H_RES, BOARD_LCD_V_RES, lvgl->disp_ready, lvgl->touch_ready);
    return ESP_OK;
}
