#include "szpi_audio.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "board_pins.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "esp_check.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "szpi_i2c.h"
#include <math.h>
#include <string.h>

static const char *TAG = "szpi_audio";
static esp_codec_dev_handle_t s_codec;
static i2s_chan_handle_t s_tx;
static i2s_chan_handle_t s_rx;

static esp_err_t szpi_i2s_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &s_tx, &s_rx), TAG, "i2s channel failed");

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = BOARD_I2S_MCK_GPIO,
            .bclk = BOARD_I2S_BCK_GPIO,
            .ws = BOARD_I2S_WS_GPIO,
            .dout = BOARD_I2S_DO_GPIO,
            .din = BOARD_I2S_DI_GPIO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(s_tx, &std_cfg), TAG, "tx init failed");
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(s_rx, &std_cfg), TAG, "rx init failed");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(s_tx), TAG, "tx enable failed");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(s_rx), TAG, "rx enable failed");
    return ESP_OK;
}

esp_err_t szpi_audio_init(void)
{
    ESP_RETURN_ON_ERROR(szpi_i2s_init(), TAG, "i2s init failed");

    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = s_rx,
        .tx_handle = s_tx,
    };
    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);
    ESP_RETURN_ON_FALSE(data_if != NULL, ESP_FAIL, TAG, "data if failed");

    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = BOARD_I2C_PORT,
        .addr = BOARD_ES8311_I2C_ADDR,
        .bus_handle = szpi_i2c_bus(),
    };
    const audio_codec_ctrl_if_t *ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    ESP_RETURN_ON_FALSE(ctrl_if != NULL, ESP_FAIL, TAG, "ctrl if failed");

    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .ctrl_if = ctrl_if,
        .gpio_if = audio_codec_new_gpio(),
        .pa_pin = BOARD_AUDIO_PA_GPIO,
        .use_mclk = true,
    };
    const audio_codec_if_t *codec_if = es8311_codec_new(&es8311_cfg);
    ESP_RETURN_ON_FALSE(codec_if != NULL, ESP_FAIL, TAG, "codec if failed");

    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = codec_if,
        .data_if = data_if,
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
    };
    s_codec = esp_codec_dev_new(&dev_cfg);
    ESP_RETURN_ON_FALSE(s_codec != NULL, ESP_FAIL, TAG, "codec dev failed");

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,
        .channel = 2,
        .bits_per_sample = 16,
    };
    ESP_RETURN_ON_ERROR(esp_codec_dev_open(s_codec, &fs), TAG, "codec open failed");
    ESP_RETURN_ON_ERROR(esp_codec_dev_set_out_vol(s_codec, 70), TAG, "volume failed");

    ESP_LOGI(TAG, "ES8311 codec ready");
    return ESP_OK;
}

esp_err_t szpi_audio_play_beep(uint32_t duration_ms)
{
    ESP_RETURN_ON_FALSE(s_codec != NULL, ESP_ERR_INVALID_STATE, TAG, "codec not ready");

    const int sample_rate = 16000;
    const int frames = (sample_rate * (int)duration_ms) / 1000;
    int16_t *samples = calloc(frames * 2, sizeof(int16_t));
    ESP_RETURN_ON_FALSE(samples != NULL, ESP_ERR_NO_MEM, TAG, "no sample buffer");

    for (int i = 0; i < frames; i++) {
        int16_t value = (int16_t)(8000 * sinf(2.0f * 3.1415926f * 880.0f * i / sample_rate));
        samples[i * 2] = value;
        samples[i * 2 + 1] = value;
    }

    esp_err_t err = esp_codec_dev_write(s_codec, samples, frames * 2 * sizeof(int16_t));
    free(samples);
    return err;
}
