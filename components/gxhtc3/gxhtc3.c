#include "gxhtc3.h"

#include <string.h>
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GXHTC3_CMD_MEASURE_T_FIRST 0x7CA2
#define GXHTC3_CMD_WAKEUP          0x3517
#define GXHTC3_CMD_SLEEP           0xB098

static esp_err_t gxhtc3_write_cmd(gxhtc3_handle_t *handle, uint16_t cmd)
{
    uint8_t buf[2] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };
    return i2c_master_transmit(handle->dev, buf, sizeof(buf), 1000);
}

static bool gxhtc3_crc8(const uint8_t *data, size_t len, uint8_t checksum)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
        }
    }
    return crc == checksum;
}

esp_err_t gxhtc3_init(gxhtc3_handle_t *handle, i2c_master_bus_handle_t bus, uint8_t addr)
{
    if (handle == NULL || bus == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 400000,
    };

    memset(handle, 0, sizeof(*handle));
    return i2c_master_bus_add_device(bus, &dev_cfg, &handle->dev);
}

esp_err_t gxhtc3_read(gxhtc3_handle_t *handle, float *temperature_c, float *humidity_rh)
{
    if (handle == NULL || handle->dev == NULL || temperature_c == NULL || humidity_rh == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(gxhtc3_write_cmd(handle, GXHTC3_CMD_WAKEUP), "gxhtc3", "wake up failed");
    vTaskDelay(pdMS_TO_TICKS(1));
    ESP_RETURN_ON_ERROR(gxhtc3_write_cmd(handle, GXHTC3_CMD_MEASURE_T_FIRST), "gxhtc3", "measure failed");
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t raw[6] = {0};
    ESP_RETURN_ON_ERROR(i2c_master_receive(handle->dev, raw, sizeof(raw), 1000), "gxhtc3", "read failed");
    ESP_RETURN_ON_ERROR(gxhtc3_write_cmd(handle, GXHTC3_CMD_SLEEP), "gxhtc3", "sleep failed");

    if (!gxhtc3_crc8(raw, 2, raw[2]) || !gxhtc3_crc8(raw + 3, 2, raw[5])) {
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t temp_raw = ((uint16_t)raw[0] << 8) | raw[1];
    uint16_t hum_raw = ((uint16_t)raw[3] << 8) | raw[4];

    *temperature_c = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
    *humidity_rh = 100.0f * ((float)hum_raw / 65535.0f);
    return ESP_OK;
}
