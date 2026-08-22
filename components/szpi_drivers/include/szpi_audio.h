#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t szpi_audio_init(void);
esp_err_t szpi_audio_play_beep(uint32_t duration_ms);

#ifdef __cplusplus
}
#endif
