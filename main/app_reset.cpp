#include "app_reset.h"

#include <esp_log.h>
#include <esp_matter.h>
#include <iot_button.h>

static const char *TAG = "app_reset";
static bool perform_factory_reset = false;

static void button_factory_reset_pressed_cb(void *arg, void *data)
{
    if (!perform_factory_reset) {
        ESP_LOGI(TAG, "Factory reset triggered. Release the button to confirm.");
        perform_factory_reset = true;
    }
}

static void button_factory_reset_released_cb(void *arg, void *data)
{
    if (perform_factory_reset) {
        ESP_LOGI(TAG, "Starting factory reset");
        esp_matter::factory_reset();
        perform_factory_reset = false;
    }
}

esp_err_t app_reset_button_register(void *handle)
{
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }

    button_handle_t button_handle = static_cast<button_handle_t>(handle);
    esp_err_t err = ESP_OK;
    err |= iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_HOLD, NULL,
                                  button_factory_reset_pressed_cb, NULL);
    err |= iot_button_register_cb(button_handle, BUTTON_PRESS_UP, NULL,
                                  button_factory_reset_released_cb, NULL);
    return err;
}
