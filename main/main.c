#include <inttypes.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "board_pins.h"

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

void app_main(void)
{
    configure_user_button();

    printf("\n");
    printf("========================================\n");
    printf("  LCKFB SZPI ESP32-C3 (立创·实战派)\n");
    printf("  Hello World from ESP-IDF\n");
    printf("========================================\n");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("Chip: ESP32-C3, %d CPU core(s), WiFi%s%s\n",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    uint32_t flash_size = 0;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        printf("Flash size: %" PRIu32 " MB\n", flash_size / (1024 * 1024));
    } else {
        printf("Flash size: unknown\n");
    }

    printf("Free heap: %lu bytes\n", esp_get_free_heap_size());
    printf("User button (GPIO%d): %d\n", BOARD_GPIO_USER_BTN,
           gpio_get_level(BOARD_GPIO_USER_BTN));
    printf("Restarting in 10 seconds...\n");

    for (int i = 10; i >= 0; i--) {
        printf("%d...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
