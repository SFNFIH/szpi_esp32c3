# SZPI GXHTC3 Matter 传感器

将板载 **GXHTC3** 温湿度传感器通过 **Matter** 接入智能家居生态（Apple Home、Google Home、Home Assistant 等），设备经 Wi-Fi 配网后可在局域网内被控制器读取。

## 架构

- **Endpoint 1**：Matter 温度传感器（Temperature Measurement 0x0402）
- **Endpoint 2**：Matter 湿度传感器（Relative Humidity Measurement 0x0405）
- 传感器数据来自现有 `szpi_env` / `gxhtc3` 驱动，每 30 秒自动上报；按 **BOOT 键（GPIO9）** 可立即刷新

## 环境要求

| 组件 | 版本 |
|------|------|
| ESP-IDF | **v5.5.x**（推荐 v5.5.5） |
| esp-matter | **release/v1.6** 或与 IDF 5.5 匹配的版本 |

## 一次性准备

```bash
# 1. 克隆 esp-matter（与 ESP-IDF 同级目录）
cd ~
git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter
git checkout release/v1.6
git submodule update --init --depth 1

# 2. 安装 esp-matter 依赖（按官方文档）
cd ~/esp-matter
./install.sh

# 3. 每次编译前加载环境
source ~/esp-matter/export.sh
source ~/esp-idf/export.sh

export ESP_MATTER_PATH=~/esp-matter
```

## 编译与烧录

```bash
cd szpi_esp32c3
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

> 设置 `ESP_MATTER_PATH` 后，工程会自动加载 `sdkconfig.defaults.matter` 与 Matter 分区表，并编译 `main/app_main.cpp`（而非 LVGL 版 `main.c`）。

烧录后屏幕会显示 **Matter 配网 QR 码**及 Manual Pairing Code；配网完成后自动切换为温湿度仪表盘。

### 屏幕显示

| 状态 | 屏幕内容 |
|------|----------|
| 未配网 | LVGL QR 码 + Manual Pairing Code |
| 已配网 | GXHTC3 温湿度实时数据 |

串口监视器仍会打印 QR 字符串，便于调试。

## 配网（Commissioning）

1. 烧录后打开串口监视器，查看 **QR 码 URL** 或 **Manual Pairing Code**
2. 在 Matter 控制器中添加设备：
   - **Apple Home**：添加配件 → 扫描 QR 码
   - **Google Home**：添加 Matter 设备
   - **Home Assistant**：设置 → 设备 → 添加集成 → Matter
3. 按提示连接 Wi-Fi 完成配网

## 恢复出厂 / 重新配网

- **长按 BOOT 键（GPIO9）约 5 秒** → 松开 → 执行 Matter Factory Reset
- 或使用串口 Shell：`matter esp factoryreset`

## chip-tool 验证（可选）

```bash
# 温度（endpoint 1）
./chip-tool temperaturemeasurement read measured-value <NODE_ID> 1

# 湿度（endpoint 2）
./chip-tool relativehumiditymeasurement read measured-value <NODE_ID> 2
```

## 与 LVGL Demo 切换

| 模式 | 条件 | 入口 |
|------|------|------|
| LVGL 仪表盘 | 不设置 `ESP_MATTER_PATH` | `main/main.c` |
| Matter 温湿度 | 设置 `ESP_MATTER_PATH` | `main/app_main.cpp` + LVGL QR |

两种模式共用 `components/gxhtc3` 驱动。Matter 模式启用 LCD/LVGL，在屏幕上显示配网 QR 码。

## 参考

- [ESP-Matter 编程指南](https://docs.espressif.com/projects/esp-matter/en/latest/esp32c3/developing.html)
- [SZPI 开发板 Wiki](https://wiki.lckfb.com/zh-hans/szpi-esp32c3/)
