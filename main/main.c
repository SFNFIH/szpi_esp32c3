#include <stdio.h>
#include "board_pins.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "szpi_lvgl.h"
#include "szpi_periph.h"

static const char *TAG = "app";

static szpi_periph_t s_periph;
static szpi_lvgl_t s_lvgl;

typedef struct {
    lv_obj_t *lbl_env;
    lv_obj_t *lbl_imu;
    lv_obj_t *lbl_touch;
    lv_obj_t *lbl_btn;
} app_ui_t;

static app_ui_t s_ui;

static void ui_update_timer_cb(lv_timer_t *timer)
{
    szpi_periph_t *periph = lv_timer_get_user_data(timer);
    char buf[128];

    if (periph->env_ready) {
        if (szpi_env_read(&periph->env) == ESP_OK) {
            snprintf(buf, sizeof(buf), "Temp: %.1f C\nHumidity: %.1f %%RH",
                     periph->env.temperature_c, periph->env.humidity_rh);
            lv_label_set_text(s_ui.lbl_env, buf);
        }
    } else {
        lv_label_set_text(s_ui.lbl_env, "ENV: N/A");
    }

    if (periph->imu_ready) {
        if (szpi_imu_read(&periph->imu) == ESP_OK) {
            snprintf(buf, sizeof(buf),
                     "Acc: %.2f, %.2f, %.2f\nGyro: %.2f, %.2f, %.2f",
                     periph->imu.accel_x, periph->imu.accel_y, periph->imu.accel_z,
                     periph->imu.gyro_x, periph->imu.gyro_y, periph->imu.gyro_z);
            lv_label_set_text(s_ui.lbl_imu, buf);
        }
    } else {
        lv_label_set_text(s_ui.lbl_imu, "IMU: N/A");
    }

    if (periph->touch_ready) {
        szpi_touch_poll(&periph->touch);
        if (periph->touch.touched) {
            snprintf(buf, sizeof(buf), "Touch: (%u, %u)", periph->touch.x, periph->touch.y);
        } else {
            snprintf(buf, sizeof(buf), "Touch: --");
        }
        lv_label_set_text(s_ui.lbl_touch, buf);
    } else {
        lv_label_set_text(s_ui.lbl_touch, "Touch: N/A");
    }

    snprintf(buf, sizeof(buf), "Button: %s",
             gpio_get_level(BOARD_GPIO_USER_BTN) == 0 ? "Pressed" : "Released");
    lv_label_set_text(s_ui.lbl_btn, buf);
}

static void app_ui_create(lv_display_t *disp, szpi_periph_t *periph)
{
    lv_obj_t *screen = lv_display_get_screen_active(disp);

    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "LCKFB SZPI ESP32-C3");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    s_ui.lbl_env = lv_label_create(screen);
    lv_obj_set_style_text_color(s_ui.lbl_env, lv_color_hex(0x7EC8E3), LV_PART_MAIN);
    lv_label_set_text(s_ui.lbl_env, "ENV: ...");
    lv_obj_align(s_ui.lbl_env, LV_ALIGN_TOP_LEFT, 8, 36);

    s_ui.lbl_imu = lv_label_create(screen);
    lv_obj_set_style_text_color(s_ui.lbl_imu, lv_color_hex(0xA8E6CF), LV_PART_MAIN);
    lv_label_set_text(s_ui.lbl_imu, "IMU: ...");
    lv_obj_align(s_ui.lbl_imu, LV_ALIGN_TOP_LEFT, 8, 96);

    s_ui.lbl_touch = lv_label_create(screen);
    lv_obj_set_style_text_color(s_ui.lbl_touch, lv_color_hex(0xFFD3B6), LV_PART_MAIN);
    lv_label_set_text(s_ui.lbl_touch, "Touch: ...");
    lv_obj_align(s_ui.lbl_touch, LV_ALIGN_TOP_LEFT, 8, 168);

    s_ui.lbl_btn = lv_label_create(screen);
    lv_obj_set_style_text_color(s_ui.lbl_btn, lv_color_hex(0xFFAAA5), LV_PART_MAIN);
    lv_label_set_text(s_ui.lbl_btn, "Button: ...");
    lv_obj_align(s_ui.lbl_btn, LV_ALIGN_TOP_LEFT, 8, 200);

    lv_timer_create(ui_update_timer_cb, 500, periph);
}

void app_main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  LCKFB SZPI ESP32-C3 (立创·实战派)\n");
    printf("  LVGL Demo\n");
    printf("========================================\n");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("Chip: ESP32-C3, %d CPU core(s), WiFi%s%s\n",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    uint32_t flash_size = 0;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        printf("Flash size: %lu MB\n", flash_size / (1024 * 1024));
    }

    ESP_ERROR_CHECK(szpi_periph_init(&s_periph));

    if (!s_periph.lcd_ready) {
        ESP_LOGE(TAG, "LCD not ready, cannot start LVGL");
        return;
    }

    ESP_ERROR_CHECK(szpi_lvgl_init(&s_lvgl, &s_periph.lcd,
                                   s_periph.touch_ready ? &s_periph.touch : NULL));

    lvgl_port_lock(0);
    app_ui_create(s_lvgl.disp, &s_periph);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "LVGL UI running");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
