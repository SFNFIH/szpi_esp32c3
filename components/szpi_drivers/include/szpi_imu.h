#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool ready;
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float temperature_c;
} szpi_imu_t;

esp_err_t szpi_imu_init(szpi_imu_t *imu);
esp_err_t szpi_imu_read(szpi_imu_t *imu);

#ifdef __cplusplus
}
#endif
