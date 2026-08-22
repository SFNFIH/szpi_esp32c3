#include "app_display.h"

#include <string.h>
#include <app/server/OnboardingCodesUtil.h>
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>

#include "board_pins.h"
#include "esp_lvgl_port.h"
#include "libs/qrcode/lv_qrcode.h"
#include "szpi_lcd.h"
#include "szpi_lvgl.h"

static const char *TAG = "app_display";

static szpi_lcd_t s_lcd;
static szpi_lvgl_t s_lvgl;
static lv_obj_t *s_comm_screen;
static lv_obj_t *s_dash_screen;
static lv_obj_t *s_qr;
static lv_obj_t *s_manual_lbl;
static lv_obj_t *s_hint_lbl;
static lv_obj_t *s_env_lbl;
static lv_obj_t *s_status_lbl;
static bool s_commissioned_view;

static esp_err_t fetch_onboarding_codes(char *qr, size_t qr_len, char *manual, size_t manual_len)
{
    chip::RendezvousInformationFlags flags;
    flags.Set(chip::RendezvousInformationFlag::kBLE);
    flags.Set(chip::RendezvousInformationFlag::kOnNetwork);

    esp_matter::lock::ScopedChipStackLock lock(portMAX_DELAY);

    chip::MutableCharSpan qr_span(qr, qr_len);
    CHIP_ERROR err = GetQRCode(qr_span, flags);
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "GetQRCode failed: %" CHIP_ERROR_FORMAT, err.Format());
        return ESP_FAIL;
    }
    qr[qr_span.size()] = '\0';

    chip::MutableCharSpan manual_span(manual, manual_len);
    err = GetManualPairingCode(manual_span, flags);
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "GetManualPairingCode failed: %" CHIP_ERROR_FORMAT, err.Format());
        return ESP_FAIL;
    }
    manual[manual_span.size()] = '\0';
    return ESP_OK;
}

static void build_commissioning_screen(void)
{
    s_comm_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_comm_screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(s_comm_screen);
    lv_label_set_text(title, "Matter Pairing");
    lv_obj_set_style_text_color(title, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    s_qr = lv_qrcode_create(s_comm_screen);
    lv_qrcode_set_size(s_qr, 140);
    lv_qrcode_set_dark_color(s_qr, lv_color_hex(0x000000));
    lv_qrcode_set_light_color(s_qr, lv_color_hex(0xFFFFFF));
    lv_obj_align(s_qr, LV_ALIGN_TOP_MID, 0, 28);
    lv_obj_set_style_border_color(s_qr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_qr, 4, LV_PART_MAIN);

    s_manual_lbl = lv_label_create(s_comm_screen);
    lv_label_set_long_mode(s_manual_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_manual_lbl, BOARD_LCD_H_RES - 16);
    lv_obj_set_style_text_align(s_manual_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_manual_lbl, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_label_set_text(s_manual_lbl, "Manual code: ----");
    lv_obj_align(s_manual_lbl, LV_ALIGN_BOTTOM_MID, 0, -36);

    s_hint_lbl = lv_label_create(s_comm_screen);
    lv_label_set_long_mode(s_hint_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_hint_lbl, BOARD_LCD_H_RES - 16);
    lv_obj_set_style_text_align(s_hint_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_hint_lbl, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_label_set_text(s_hint_lbl, "Scan QR with Home app");
    lv_obj_align(s_hint_lbl, LV_ALIGN_BOTTOM_MID, 0, -8);
}

static void build_dashboard_screen(void)
{
    s_dash_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_dash_screen, lv_color_hex(0x101820), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(s_dash_screen);
    lv_label_set_text(title, "SZPI Matter Sensor");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    s_env_lbl = lv_label_create(s_dash_screen);
    lv_obj_set_style_text_color(s_env_lbl, lv_color_hex(0x7EC8E3), LV_PART_MAIN);
    lv_label_set_text(s_env_lbl, "GXHTC3\nTemp: --\nHumidity: --");
    lv_obj_align(s_env_lbl, LV_ALIGN_CENTER, 0, -20);

    s_status_lbl = lv_label_create(s_dash_screen);
    lv_obj_set_style_text_color(s_status_lbl, lv_color_hex(0xA8E6CF), LV_PART_MAIN);
    lv_label_set_text(s_status_lbl, "Commissioned");
    lv_obj_align(s_status_lbl, LV_ALIGN_BOTTOM_MID, 0, -12);
}

esp_err_t app_display_init(void)
{
    ESP_RETURN_ON_ERROR(szpi_lcd_init(&s_lcd), TAG, "lcd init failed");
    ESP_RETURN_ON_ERROR(szpi_lvgl_init(&s_lvgl, &s_lcd, NULL), TAG, "lvgl init failed");

    lvgl_port_lock(0);
    build_commissioning_screen();
    build_dashboard_screen();
    lv_screen_load(s_comm_screen);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "Display ready (%dx%d)", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return ESP_OK;
}

esp_err_t app_display_show_commissioning(void)
{
    char qr[128] = {0};
    char manual[32] = {0};

    if (fetch_onboarding_codes(qr, sizeof(qr), manual, sizeof(manual)) != ESP_OK) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Onboarding QR: %s", qr);
    ESP_LOGI(TAG, "Manual code: %s", manual);

    lvgl_port_lock(0);
    if (lv_qrcode_update(s_qr, qr, strlen(qr)) != LV_RESULT_OK) {
        lvgl_port_unlock();
        ESP_LOGE(TAG, "lv_qrcode_update failed");
        return ESP_FAIL;
    }

    char manual_text[48];
    snprintf(manual_text, sizeof(manual_text), "Manual: %s", manual);
    lv_label_set_text(s_manual_lbl, manual_text);
    lv_screen_load(s_comm_screen);
    s_commissioned_view = false;
    lvgl_port_unlock();
    return ESP_OK;
}

esp_err_t app_display_show_dashboard(void)
{
    lvgl_port_lock(0);
    lv_screen_load(s_dash_screen);
    s_commissioned_view = true;
    lvgl_port_unlock();
    return ESP_OK;
}

void app_display_update_sensors(float temp_c, float humidity_rh)
{
    if (!s_commissioned_view || s_env_lbl == NULL) {
        return;
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "GXHTC3\nTemp: %.1f C\nHumidity: %.1f %%RH", temp_c, humidity_rh);

    lvgl_port_lock(0);
    lv_label_set_text(s_env_lbl, buf);
    lvgl_port_unlock();
}

bool app_display_is_commissioned_view(void)
{
    return s_commissioned_view;
}
