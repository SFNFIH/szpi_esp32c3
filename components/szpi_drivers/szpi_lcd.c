#include "szpi_lcd.h"

#include "board_pins.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "szpi_lcd";

esp_err_t szpi_lcd_set_backlight(bool on)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << BOARD_LCD_BK_LIGHT,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "backlight gpio failed");
    gpio_set_level(BOARD_LCD_BK_LIGHT, on ? 0 : 1);
    return ESP_OK;
}

esp_err_t szpi_lcd_init(szpi_lcd_t *lcd)
{
    ESP_RETURN_ON_FALSE(lcd != NULL, ESP_ERR_INVALID_ARG, TAG, "lcd is null");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = BOARD_LCD_SPI_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = BOARD_LCD_SPI_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BOARD_LCD_H_RES * BOARD_LCD_V_RES * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO), TAG, "spi init failed");

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = BOARD_LCD_SPI_CS,
        .dc_gpio_num = BOARD_LCD_SPI_DC,
        .spi_mode = 2,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_cfg, &lcd->io), TAG, "panel io failed");

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = GPIO_NUM_NC,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(lcd->io, &panel_cfg, &lcd->panel), TAG, "panel failed");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(lcd->panel), TAG, "panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(lcd->panel), TAG, "panel init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(lcd->panel, true), TAG, "invert failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(lcd->panel, true), TAG, "swap failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(lcd->panel, true, false), TAG, "mirror failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(lcd->panel, true), TAG, "disp on failed");
    ESP_RETURN_ON_ERROR(szpi_lcd_set_backlight(true), TAG, "backlight failed");

    ESP_LOGI(TAG, "ST7789 LCD ready (%dx%d)", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return ESP_OK;
}

esp_err_t szpi_lcd_fill_color(szpi_lcd_t *lcd, uint16_t color565)
{
    ESP_RETURN_ON_FALSE(lcd != NULL && lcd->panel != NULL, ESP_ERR_INVALID_STATE, TAG, "lcd not ready");

    const size_t pixels = BOARD_LCD_H_RES * BOARD_LCD_V_RES;
    uint16_t *buffer = heap_caps_malloc(pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
    ESP_RETURN_ON_FALSE(buffer != NULL, ESP_ERR_NO_MEM, TAG, "no dma buffer");

    for (size_t i = 0; i < pixels; i++) {
        buffer[i] = color565;
    }

    esp_err_t err = esp_lcd_panel_draw_bitmap(lcd->panel, 0, 0, BOARD_LCD_H_RES, BOARD_LCD_V_RES, buffer);
    heap_caps_free(buffer);
    return err;
}
