# GXHTC3

ESP-IDF driver for the **GXHTC3** digital temperature and humidity sensor.

The GXHTC3 is used on the [LCKFB SZPI ESP32-C3](https://wiki.lckfb.com/zh-hans/szpi-esp32c3/) board (I2C address `0x70`). This component uses the ESP-IDF 5.x **I2C master** driver API.

## Features

- Wake / measure / read / sleep command sequence
- CRC8 validation of sensor data
- Default I2C address `0x70` (`GXHTC3_I2C_ADDR_DEFAULT`)

## Add as dependency

```bash
idf.py add-dependency "SFNFIH/gxhtc3^0.1.0"
```

Or in `idf_component.yml`:

```yaml
dependencies:
  SFNFIH/gxhtc3: "^0.1.0"
```

## Usage

```c
#include "gxhtc3.h"

gxhtc3_handle_t sensor;
ESP_ERROR_CHECK(gxhtc3_init(&sensor, i2c_bus, GXHTC3_I2C_ADDR_DEFAULT));

float temp_c = 0.0f;
float rh = 0.0f;
ESP_ERROR_CHECK(gxhtc3_read(&sensor, &temp_c, &rh));
printf("Temperature: %.1f C, Humidity: %.1f %%RH\n", temp_c, rh);
```

## Requirements

- ESP-IDF >= 5.0
- Shared I2C master bus (`i2c_master_bus_handle_t`) already initialized

## License

MIT — see [LICENSE](LICENSE).
