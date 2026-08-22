#include "szpi_env.h"

#include "board_pins.h"
#include "esp_check.h"
#include "esp_log.h"
#include "gxhtc3.h"
#include "szpi_i2c.h"

static const char *TAG = "szpi_env";
static gxhtc3_handle_t s_gxhtc3;

esp_err_t szpi_env_init(szpi_env_t *env)
{
    ESP_RETURN_ON_FALSE(env != NULL, ESP_ERR_INVALID_ARG, TAG, "env is null");
    ESP_RETURN_ON_ERROR(gxhtc3_init(&s_gxhtc3, szpi_i2c_bus(), BOARD_GXHTC3_I2C_ADDR), TAG, "gxhtc3 init failed");
    env->ready = true;
    ESP_LOGI(TAG, "GXHTC3 sensor ready (addr=0x%02X)", BOARD_GXHTC3_I2C_ADDR);
    return ESP_OK;
}

esp_err_t szpi_env_read(szpi_env_t *env)
{
    ESP_RETURN_ON_FALSE(env != NULL && env->ready, ESP_ERR_INVALID_STATE, TAG, "env not ready");
    ESP_RETURN_ON_ERROR(gxhtc3_read(&s_gxhtc3, &env->temperature_c, &env->humidity_rh), TAG, "read failed");
    return ESP_OK;
}
