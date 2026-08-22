#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <esp_matter.h>
#include <esp_matter_console.h>

#include <app_priv.h>
#include <app_reset.h>
#include <app_display.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

static const char *TAG = "app_main";

uint16_t g_temperature_endpoint_id = 0;
uint16_t g_humidity_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_commissioning_timeout_seconds = 300;

static void app_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP address changed");
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        app_display_show_dashboard();
        app_driver_update_matter_values();
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        app_display_show_commissioning();
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        ESP_LOGI(TAG, "Fabric removed");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commission_mgr =
                chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto timeout = chip::System::Clock::Seconds16(k_commissioning_timeout_seconds);
            if (!commission_mgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commission_mgr.OpenBasicCommissioningWindow(
                    timeout, chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG, "Failed to reopen commissioning window");
                }
            }
        }
        break;
    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id,
                                         uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identify: endpoint=%u effect=%u", endpoint_id, effect_id);
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id,
                                         uint32_t cluster_id, uint32_t attribute_id,
                                         esp_matter_attr_val_t *val, void *priv_data)
{
    (void)endpoint_id;
    (void)cluster_id;
    (void)attribute_id;
    (void)val;
    (void)priv_data;
    (void)type;
    return ESP_OK;
}

extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(app_display_init());

    app_driver_handle_t sensor_handle = app_driver_sensor_init();
    app_driver_handle_t button_handle = app_driver_button_init();
    if (button_handle) {
        app_reset_button_register(button_handle);
    }

    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return;
    }

    temperature_sensor::config_t temp_cfg;
    temp_cfg.temperature_measurement.measured_value =
        esp_matter_nullable_int16(DEFAULT_TEMPERATURE_VALUE);
    endpoint_t *temp_ep = temperature_sensor::create(node, &temp_cfg, ENDPOINT_FLAG_NONE, sensor_handle);
    if (!temp_ep) {
        ESP_LOGE(TAG, "Failed to create temperature endpoint");
        return;
    }
    g_temperature_endpoint_id = endpoint::get_id(temp_ep);
    ESP_LOGI(TAG, "Temperature endpoint id=%u", g_temperature_endpoint_id);

    humidity_sensor::config_t hum_cfg;
    hum_cfg.relative_humidity_measurement.measured_value =
        esp_matter_nullable_uint16(DEFAULT_HUMIDITY_VALUE);
    endpoint_t *hum_ep = humidity_sensor::create(node, &hum_cfg, ENDPOINT_FLAG_NONE, nullptr);
    if (!hum_ep) {
        ESP_LOGE(TAG, "Failed to create humidity endpoint");
        return;
    }
    g_humidity_endpoint_id = endpoint::get_id(hum_ep);
    ESP_LOGI(TAG, "Humidity endpoint id=%u", g_humidity_endpoint_id);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t ot_config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&ot_config);
#endif

    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Matter start failed: %s", esp_err_to_name(err));
        return;
    }

    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0) {
        app_display_show_dashboard();
    } else {
        app_display_show_commissioning();
    }

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::attribute_register_commands();
    esp_matter::console::init();
#endif

    ESP_LOGI(TAG, "SZPI GXHTC3 Matter sensor running");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
