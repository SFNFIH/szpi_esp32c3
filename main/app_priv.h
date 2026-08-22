#pragma once

#include <esp_err.h>
#include <esp_matter.h>

typedef void *app_driver_handle_t;

#define DEFAULT_TEMPERATURE_VALUE 2000
#define DEFAULT_HUMIDITY_VALUE 5000
#define GXHTC3_MEASURE_INTERVAL_MS 30000

app_driver_handle_t app_driver_sensor_init(void);
app_driver_handle_t app_driver_button_init(void);

esp_err_t app_driver_update_matter_values(void);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG() \
    { \
        .radio_mode = RADIO_MODE_NATIVE, \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG() \
    { \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE, \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG() \
    { \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif
