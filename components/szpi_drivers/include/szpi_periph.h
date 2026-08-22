#pragma once

#include "esp_err.h"
#include "szpi_env.h"
#include "szpi_imu.h"
#include "szpi_lcd.h"
#include "szpi_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    szpi_lcd_t lcd;
    szpi_touch_t touch;
    szpi_imu_t imu;
    szpi_env_t env;
    bool lcd_ready;
    bool touch_ready;
    bool imu_ready;
    bool env_ready;
    bool audio_ready;
} szpi_periph_t;

esp_err_t szpi_periph_init(szpi_periph_t *periph);
void szpi_periph_demo_loop(szpi_periph_t *periph);

#ifdef __cplusplus
}
#endif
