# AI-Biodome 智能大棚监控系统

基于 ESP32-S3 的 AI 智能大棚环境监控与自动控制系统。

## 功能

- **传感器监测**: 温湿度(SHT40)、光照(BH1750)、空气质量(SGP30/eCO2+TVOC)、土壤湿度
- **自动控制**: 4路继电器(水泵/补光灯/加热垫/风扇)，支持自动/手动模式切换
- **定时调度**: 风扇通风和补光灯的每日定时任务
- **WebSocket 实时推送**: 5秒间隔推送全量环境数据
- **历史图表**: ECharts 可视化近60条历史数据（温湿度/光照土壤/空气质量）
- **AI 环境分析**: 基于实时数据的大棚状态分析与农业建议
- **WiFi AP配网**: 首次启动自动进入AP模式，扫码配网

## 硬件清单

| 组件 | 型号 | 接口 |
|------|------|------|
| 主控 | ESP32-S3-DevKitC-1 | — |
| 温湿度传感器 | SHT40 | I2C (SDA=4, SCL=5, addr=0x44) |
| 光照传感器 | BH1750 | I2C (同上总线) |
| 空气质量传感器 | SGP30 | I2C (同上总线) |
| 土壤湿度传感器 | 模拟量输出 | GPIO6 (ADC) |
| RGB LED | WS2812B | GPIO48 |
| 继电器×4 | 低电平触发 | GPIO8/9/10/11 |

## 引脚定义

```
GPIO4  → I2C SDA (SHT40 + BH1750 + SGP30)
GPIO5  → I2C SCL
GPIO6  → 土壤湿度传感器 (ADC)
GPIO8  → 继电器1 - 水泵
GPIO9  → 继电器2 - 补光灯
GPIO10 → 继电器3 - 加热垫
GPIO11 → 继电器4 - 风扇
GPIO48 → 板载 WS2812B RGB LED
```

## 编译方法

### 使用 PlatformIO

```bash
# 安装 PlatformIO CLI
pip install platformio

# 编译
pio run

# 编译并上传
pio run -t upload

# 串口监视
pio device monitor
```

### 使用 Arduino IDE

1. 安装 ESP32 开发板支持 (boards manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)
2. 选择开发板: `ESP32S3 Dev Module`
3. 安装依赖库:
   - Adafruit NeoPixel
   - Adafruit SGP30 Sensor
   - Adafruit BH1750
   - ArduinoJson
   - WebSockets
4. 编译上传

## WiFi 配置

1. 首次启动或配置重置后，设备自动创建 AP 热点 `AI-Biodome-Config`
2. 手机连接该热点，浏览器自动弹出配网页面
3. 选择 WiFi 并输入密码，设备自动重启并连接
4. 连接成功后，串口输出 IP 地址，浏览器访问该 IP 即可查看控制面板
5. 访问 `/reset` 可清除 WiFi 配置并重新配网

## API 说明

### WebSocket (端口 81)

连接后自动推送全量 JSON 数据，5秒刷新。

**客户端发送指令:**

```json
// 切换模式
{"action": "set_mode", "mode": "auto"}

// 手动控制设备
{"action": "set_device", "device": "pump", "state": 1}

// 设置定时任务
{"action": "set_sched", "fan_en": true, "fan_start": "08:00", "fan_end": "18:00", "light_en": true, "light_start": "06:00", "light_end": "20:00"}
```

### HTTP

- `GET /` — 控制面板页面
- `GET /reset` — 清除 WiFi 配置并重启

## 自动控制逻辑

| 设备 | 开启条件 | 关闭条件 |
|------|---------|---------|
| 水泵 | 土壤湿度 < 30% | 土壤湿度 > 60% |
| 补光灯 | 光照 < 200lx 或 定时开启 | 光照 > 800lx 且非定时 |
| 加热垫 | 温度 < 18°C | 温度 > 22°C |
| 风扇 | 温度 > 28°C 或 eCO2 > 1000ppm 或 定时开启 | 温度 < 25°C 且 eCO2 < 800ppm 且非定时 |

## License

MIT
