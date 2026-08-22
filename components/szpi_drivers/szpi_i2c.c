#include "szpi_i2c.h"

#include "board_pins.h"
#include "esp_check.h"

static i2c_master_bus_handle_t s_i2c_bus;

esp_err_t szpi_i2c_init(i2c_master_bus_handle_t *bus)
{
    if (s_i2c_bus != NULL) {
        if (bus) {
            *bus = s_i2c_bus;
        }
        return ESP_OK;
    }

    i2c_master_bus_config_t cfg = {
        .i2c_port = BOARD_I2C_PORT,
        .sda_io_num = BOARD_I2C_SDA_GPIO,
        .scl_io_num = BOARD_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&cfg, &s_i2c_bus), "szpi_i2c", "create bus failed");
    if (bus) {
        *bus = s_i2c_bus;
    }
    return ESP_OK;
}

i2c_master_bus_handle_t szpi_i2c_bus(void)
{
    return s_i2c_bus;
}
