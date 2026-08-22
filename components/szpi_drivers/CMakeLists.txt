set(SZPI_SRCS "szpi_i2c.c" "szpi_env.c")

if(NOT CONFIG_SZPI_MATTER_MINIMAL)
    list(APPEND SZPI_SRCS
         "szpi_lcd.c"
         "szpi_touch.c"
         "szpi_imu.c"
         "szpi_audio.c"
         "szpi_periph.c"
         "szpi_lvgl.c")
endif()

set(SZPI_REQUIRES board gxhtc3
                  esp_driver_i2c esp_timer)

if(NOT CONFIG_SZPI_MATTER_MINIMAL)
    list(APPEND SZPI_REQUIRES esp_lvgl_port
                              esp_driver_spi esp_driver_gpio
                              esp_driver_i2s esp_lcd)
endif()

set(SZPI_PRIV_REQUIRES "")
if(NOT CONFIG_SZPI_MATTER_MINIMAL)
    list(APPEND SZPI_PRIV_REQUIRES esp_lcd_touch_cst816s esp_codec_dev)
endif()

idf_component_register(SRCS ${SZPI_SRCS}
                    INCLUDE_DIRS "include"
                    REQUIRES ${SZPI_REQUIRES}
                    PRIV_REQUIRES ${SZPI_PRIV_REQUIRES})
