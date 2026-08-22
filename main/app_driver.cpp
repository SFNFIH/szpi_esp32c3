#include "app_priv.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <iot_button.h>

#include <esp_matter.h>

#include "board_pins.h"
#include "szpi_env.h"
#include "szpi_i2c.h"
#include "app_display.h"

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::attribute;

static const char *TAG = "app_driver";

extern uint16_t g_temperature_endpoint_id;
extern uint16_t g_humidity_endpoint_id;

static szpi_env_t s_env;

static esp_err_t update_matter_from_env(void)
{
    if (!s_env.ready) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = szpi_env_read(&s_env);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "GXHTC3 read failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_matter_attr_val_t temp_val = esp_matter_nullable_int16(
        static_cast<int16_t>(s_env.temperature_c * 100.0f));
    esp_matter_attr_val_t hum_val = esp_matter_nullable_uint16(
        static_cast<uint16_t>(s_env.humidity_rh * 100.0f));

    err = attribute::update(g_temperature_endpoint_id, TemperatureMeasurement::Id,
                            TemperatureMeasurement::Attributes::MeasuredValue::Id, &temp_val);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Temperature attribute update failed");
        return err;
    }

    err = attribute::update(g_humidity_endpoint_id, RelativeHumidityMeasurement::Id,
                            RelativeHumidityMeasurement::Attributes::MeasuredValue::Id, &hum_val);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Humidity attribute update failed");
        return err;
    }

    ESP_LOGI(TAG, "Matter updated: temp=%.1f C humidity=%.1f %%RH",
             s_env.temperature_c, s_env.humidity_rh);
    app_display_update_sensors(s_env.temperature_c, s_env.humidity_rh);
    return ESP_OK;
}

esp_err_t app_driver_update_matter_values(void)
{
    return update_matter_from_env();
}

static void gxhtc3_task(void *arg)
{
    (void)arg;

    for (;;) {
        update_matter_from_env();
        vTaskDelay(pdMS_TO_TICKS(GXHTC3_MEASURE_INTERVAL_MS));
    }
}

static void button_refresh_cb(void *arg, void *data)
{
    (void)arg;
    (void)data;
    ESP_LOGI(TAG, "Button pressed, refreshing sensor values");
    update_matter_from_env();
}

app_driver_handle_t app_driver_sensor_init(void)
{
    ESP_ERROR_CHECK(szpi_i2c_init(NULL));

    if (szpi_env_init(&s_env) != ESP_OK) {
        ESP_LOGE(TAG, "GXHTC3 init failed");
        return nullptr;
    }

    xTaskCreate(gxhtc3_task, "gxhtc3_matter", 4096, NULL, 5, NULL);
    update_matter_from_env();
    return reinterpret_cast<app_driver_handle_t>(&s_env);
}

app_driver_handle_t app_driver_button_init(void)
{
    const button_config_t btn_cfg = {};
    const button_gpio_config_t gpio_cfg = {
        .gpio_num = BOARD_GPIO_USER_BTN,
        .active_level = 0,
        .enable_power_save = false,
    };

    button_handle_t handle = NULL;
    if (iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &handle) != ESP_OK) {
        ESP_LOGW(TAG, "Button init failed");
        return nullptr;
    }

    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, NULL, button_refresh_cb, NULL);
    return reinterpret_cast<app_driver_handle_t>(handle);
}
