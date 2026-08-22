#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool ready;
    float temperature_c;
    float humidity_rh;
} szpi_env_t;

esp_err_t szpi_env_init(szpi_env_t *env);
esp_err_t szpi_env_read(szpi_env_t *env);

#ifdef __cplusplus
}
#endif
