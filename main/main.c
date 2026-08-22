#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board_pins.h"
#include "szpi_periph.h"

static const char *TAG = "app";
static szpi_periph_t s_periph;

void app_main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  LCKFB SZPI ESP32-C3 (立创·实战派)\n");
    printf("  Peripheral Demo (ESP Component Registry)\n");
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
    ESP_LOGI(TAG, "All peripherals initialized, entering demo loop");

    while (true) {
        szpi_periph_demo_loop(&s_periph);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
