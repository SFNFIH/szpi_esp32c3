#include "szpi_imu.h"

#include "board_pins.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "szpi_i2c.h"

static const char *TAG = "szpi_imu";

#define QMI8658_REG_WHO_AM_I 0x00
#define QMI8658_REG_CTRL1    0x02
#define QMI8658_REG_CTRL2    0x03
#define QMI8658_REG_CTRL3    0x04
#define QMI8658_REG_CTRL7    0x08
#define QMI8658_REG_AX_L       0x35

static i2c_master_dev_handle_t s_imu_dev;

static esp_err_t qmi8658_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
    return i2c_master_transmit(s_imu_dev, buf, sizeof(buf), 1000);
}

static esp_err_t qmi8658_read_reg(uint8_t reg, uint8_t *value)
{
    return i2c_master_transmit_receive(s_imu_dev, &reg, 1, value, 1, 1000);
}

static esp_err_t qmi8658_read_bytes(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(s_imu_dev, &reg, 1, data, len, 1000);
}

esp_err_t szpi_imu_init(szpi_imu_t *imu)
{
    ESP_RETURN_ON_FALSE(imu != NULL, ESP_ERR_INVALID_ARG, TAG, "imu is null");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BOARD_IMU_I2C_ADDR,
        .scl_speed_hz = BOARD_I2C_FREQ_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(szpi_i2c_bus(), &dev_cfg, &s_imu_dev), TAG, "add dev failed");

    uint8_t who_am_i = 0;
    ESP_RETURN_ON_ERROR(qmi8658_read_reg(QMI8658_REG_WHO_AM_I, &who_am_i), TAG, "whoami read failed");
    ESP_RETURN_ON_FALSE(who_am_i == 0x05, ESP_ERR_NOT_FOUND, TAG, "unexpected whoami 0x%02X", who_am_i);

    ESP_RETURN_ON_ERROR(qmi8658_write_reg(QMI8658_REG_CTRL1, 0x60), TAG, "ctrl1 failed");
    ESP_RETURN_ON_ERROR(qmi8658_write_reg(QMI8658_REG_CTRL7, 0x03), TAG, "ctrl7 failed");
    ESP_RETURN_ON_ERROR(qmi8658_write_reg(QMI8658_REG_CTRL2, 0x95), TAG, "ctrl2 failed");
    ESP_RETURN_ON_ERROR(qmi8658_write_reg(QMI8658_REG_CTRL3, 0xD5), TAG, "ctrl3 failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    imu->ready = true;
    ESP_LOGI(TAG, "QMI8658C IMU ready (addr=0x%02X, whoami=0x%02X)", BOARD_IMU_I2C_ADDR, who_am_i);
    return ESP_OK;
}

esp_err_t szpi_imu_read(szpi_imu_t *imu)
{
    ESP_RETURN_ON_FALSE(imu != NULL && imu->ready, ESP_ERR_INVALID_STATE, TAG, "imu not ready");

    uint8_t raw[12] = {0};
    ESP_RETURN_ON_ERROR(qmi8658_read_bytes(QMI8658_REG_AX_L, raw, sizeof(raw)), TAG, "read failed");

    int16_t ax = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t ay = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t az = (int16_t)((raw[5] << 8) | raw[4]);
    int16_t gx = (int16_t)((raw[7] << 8) | raw[6]);
    int16_t gy = (int16_t)((raw[9] << 8) | raw[8]);
    int16_t gz = (int16_t)((raw[11] << 8) | raw[10]);

    imu->accel_x = ax / 4096.0f;
    imu->accel_y = ay / 4096.0f;
    imu->accel_z = az / 4096.0f;
    imu->gyro_x = gx / 64.0f * 3.1415926f / 180.0f;
    imu->gyro_y = gy / 64.0f * 3.1415926f / 180.0f;
    imu->gyro_z = gz / 64.0f * 3.1415926f / 180.0f;
    imu->temperature_c = 0.0f;
    return ESP_OK;
}
