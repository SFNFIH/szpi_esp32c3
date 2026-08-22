#include "szpi_touch.h"

#include "board_pins.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "szpi_i2c.h"

static const char *TAG = "szpi_touch";
static SemaphoreHandle_t s_touch_mux;

static void touch_isr_cb(esp_lcd_touch_handle_t tp)
{
    BaseType_t wake = pdFALSE;
    if (s_touch_mux) {
        xSemaphoreGiveFromISR(s_touch_mux, &wake);
    }
    if (wake == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

esp_err_t szpi_touch_init(szpi_touch_t *touch)
{
    ESP_RETURN_ON_FALSE(touch != NULL, ESP_ERR_INVALID_ARG, TAG, "touch is null");

    if (s_touch_mux == NULL) {
        s_touch_mux = xSemaphoreCreateBinary();
        ESP_RETURN_ON_FALSE(s_touch_mux != NULL, ESP_ERR_NO_MEM, TAG, "touch mutex failed");
    }

    esp_lcd_panel_io_handle_t tp_io = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_cfg = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    tp_io_cfg.scl_speed_hz = BOARD_I2C_FREQ_HZ;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(szpi_i2c_bus(), &tp_io_cfg, &tp_io), TAG, "touch io failed");

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = BOARD_LCD_H_RES,
        .y_max = BOARD_LCD_V_RES,
        .rst_gpio_num = BOARD_TOUCH_RST_GPIO,
        .int_gpio_num = BOARD_TOUCH_INT_GPIO,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 1,
            .mirror_y = 0,
        },
        .interrupt_callback = touch_isr_cb,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst816s(tp_io, &tp_cfg, &touch->handle), TAG, "touch init failed");
    touch->touched = false;
    touch->x = 0;
    touch->y = 0;
    ESP_LOGI(TAG, "CST816S touch ready (INT=GPIO%d, RST=GPIO%d)", BOARD_TOUCH_INT_GPIO, BOARD_TOUCH_RST_GPIO);
    return ESP_OK;
}

esp_err_t szpi_touch_poll(szpi_touch_t *touch)
{
    ESP_RETURN_ON_FALSE(touch != NULL && touch->handle != NULL, ESP_ERR_INVALID_STATE, TAG, "touch not ready");

    touch->touched = false;
    if (xSemaphoreTake(s_touch_mux, 0) != pdTRUE) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(esp_lcd_touch_read_data(touch->handle), TAG, "read failed");

    esp_lcd_touch_point_data_t point = {0};
    uint8_t count = 0;
    ESP_RETURN_ON_ERROR(esp_lcd_touch_get_data(touch->handle, &point, &count, 1), TAG, "get data failed");
    if (count > 0) {
        touch->touched = true;
        touch->x = point.x;
        touch->y = point.y;
    }
    return ESP_OK;
}
