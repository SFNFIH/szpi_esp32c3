#include "szpi_periph.h"

#include <string.h>
#include "board_pins.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "szpi_audio.h"
#include "szpi_i2c.h"

static const char *TAG = "szpi_periph";

static void configure_user_button(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << BOARD_GPIO_USER_BTN,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
}

esp_err_t szpi_periph_init(szpi_periph_t *periph)
{
    ESP_RETURN_ON_FALSE(periph != NULL, ESP_ERR_INVALID_ARG, TAG, "periph is null");
    memset(periph, 0, sizeof(*periph));

    configure_user_button();
    ESP_RETURN_ON_ERROR(szpi_i2c_init(NULL), TAG, "i2c init failed");

    if (szpi_lcd_init(&periph->lcd) == ESP_OK) {
        periph->lcd_ready = true;
    } else {
        ESP_LOGW(TAG, "LCD init failed");
    }

    if (szpi_touch_init(&periph->touch) == ESP_OK) {
        periph->touch_ready = true;
    } else {
        ESP_LOGW(TAG, "Touch init failed");
    }

    if (szpi_imu_init(&periph->imu) == ESP_OK) {
        periph->imu_ready = true;
    } else {
        ESP_LOGW(TAG, "IMU init failed");
    }

    if (szpi_env_init(&periph->env) == ESP_OK) {
        periph->env_ready = true;
    } else {
        ESP_LOGW(TAG, "Environment sensor init failed");
    }

    if (szpi_audio_init() == ESP_OK) {
        periph->audio_ready = true;
        szpi_audio_play_beep(120);
    } else {
        ESP_LOGW(TAG, "Audio init failed");
    }

    ESP_LOGI(TAG, "Peripheral init summary: LCD=%d Touch=%d IMU=%d ENV=%d Audio=%d",
             periph->lcd_ready, periph->touch_ready, periph->imu_ready,
             periph->env_ready, periph->audio_ready);
    return ESP_OK;
}
