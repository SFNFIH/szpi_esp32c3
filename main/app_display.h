#pragma once

#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_display_init(void);
esp_err_t app_display_show_commissioning(void);
esp_err_t app_display_show_dashboard(void);
void app_display_update_sensors(float temp_c, float humidity_rh);
bool app_display_is_commissioned_view(void);

#ifdef __cplusplus
}
#endif
