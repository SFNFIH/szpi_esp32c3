#pragma once

#include "driver/i2c_types.h"

/**
 * Pin definitions for LCKFB SZPI ESP32-C3 (立创·实战派 ESP32-C3)
 * Reference: https://wiki.lckfb.com/zh-hans/szpi-esp32c3/
 */

/* User button (BOOT key on this board) */
#define BOARD_GPIO_USER_BTN         9

/* Shared I2C bus: touch, sensors (GXHTC3, QMI8658C, ES8311, etc.) */
#define BOARD_I2C_PORT              I2C_NUM_0
#define BOARD_I2C_SCL_GPIO          1
#define BOARD_I2C_SDA_GPIO          0
#define BOARD_I2C_FREQ_HZ           400000

/* CST816S capacitive touch (I2C) */
#define BOARD_TOUCH_I2C_ADDR        0x15
#define BOARD_TOUCH_INT_GPIO        14
#define BOARD_TOUCH_RST_GPIO        15

/* QMI8658C 6-axis IMU (I2C) */
#define BOARD_IMU_I2C_ADDR          0x6B

/* GXHTC3 temperature & humidity (I2C) */
#define BOARD_GXHTC3_I2C_ADDR       0x70

/* ES8311 audio codec (I2C control) */
#define BOARD_ES8311_I2C_ADDR       0x18

/* LCD (ST7789, SPI) */
#define BOARD_LCD_SPI_SCLK          3
#define BOARD_LCD_SPI_MOSI          5
#define BOARD_LCD_SPI_CS            4
#define BOARD_LCD_SPI_DC            6
#define BOARD_LCD_BK_LIGHT          2
#define BOARD_LCD_H_RES             240
#define BOARD_LCD_V_RES             320

/* I2S audio (ES8311) */
#define BOARD_I2S_MCK_GPIO          10
#define BOARD_I2S_BCK_GPIO          8
#define BOARD_I2S_WS_GPIO           12
#define BOARD_I2S_DO_GPIO           11  /* Requires eFuse: VDD_SPI as GPIO */
#define BOARD_I2S_DI_GPIO           7
#define BOARD_AUDIO_PA_GPIO         13

/* Extension header */
#define BOARD_EXT_GPIO_18           18
#define BOARD_EXT_GPIO_19           19
