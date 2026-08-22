idf_component_register(SRCS "szpi_i2c.c"
                           "szpi_lcd.c"
                           "szpi_touch.c"
                           "szpi_imu.c"
                           "szpi_env.c"
                           "szpi_audio.c"
                           "szpi_periph.c"
                           "szpi_lvgl.c"
                    INCLUDE_DIRS "include"
                    REQUIRES board gxhtc3 esp_lvgl_port
                             esp_driver_i2c esp_driver_spi esp_driver_gpio
                             esp_driver_i2s esp_lcd esp_timer
                    PRIV_REQUIRES esp_lcd_touch_cst816s esp_codec_dev)
