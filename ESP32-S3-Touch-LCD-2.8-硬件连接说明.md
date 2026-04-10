# ESP32-S3-Touch-LCD-2.8 硬件连接说明

## 1. 文档目的

这份文档用于把 Waveshare `ESP32-S3-Touch-LCD-2.8` 的官方 Wiki、资源页/原理图，与本地 `ESP-IDF` 官方例程中的实际初始化代码对应起来，方便后续做二次开发、移植驱动、排查引脚冲突。

## 2. 参考来源

- 官方 Wiki: <https://docs.waveshare.net/ESP32-S3-Touch-LCD-2.8/>
- 资源页: <https://docs.waveshare.net/ESP32-S3-Touch-LCD-2.8/Resources-And-Documents/>
- 原理图 PDF: <https://www.waveshare.net/w/upload/3/33/ESP32-S3-Touch-LCD-2.8.pdf>
- 本地例程: `E:\gitku\xinpan\ESP-IDF\ESP32-S3-Touch-LCD-2.8-Test`

说明:
- 下文中“官方文档确认”表示来自 Wiki/资源页/原理图。
- 下文中“例程确认”表示来自本地 `ESP-IDF` 工程实际代码。
- 少量结论是根据“原理图信号名 + 例程引脚定义”交叉推断得出，我会单独标出来。

## 3. 板载资源总览

官方 Wiki 显示，这块板的核心资源包括:

- ESP32-S3 主控
- 2.8 英寸 240x320 电容触摸屏
- 板载 RTC
- 六轴 IMU
- 音频 DAC 与喇叭接口
- MicroSD 卡槽
- 电池接口与电量检测
- Wi-Fi / BLE 无线能力

从本地例程来看，官方示例已经把这些资源基本都串起来了，形成了一个完整的“板级测试/演示工程”。

## 4. 板级连接关系

### 4.1 LCD 显示屏 ST7789T

例程文件:
- `main/LCD_Driver/ST7789.h`
- `main/LCD_Driver/ST7789.c`
- `main/LCD_Driver/Vernon_ST7789T/Vernon_ST7789T.c`

接口方式:
- SPI3

引脚映射:

| 功能 | GPIO | 说明 |
| --- | ---: | --- |
| LCD_SCLK | 40 | SPI 时钟 |
| LCD_MOSI | 45 | SPI 数据输出 |
| LCD_CS | 42 | 片选 |
| LCD_DC | 41 | 数据/命令选择 |
| LCD_RST | 39 | 面板复位 |
| LCD_BL | 5 | 背光 PWM 控制 |
| LCD_MISO | 未使用 | 例程中设为 `-1` |

关键参数:

- 分辨率: `240 x 320`
- 像素时钟: `80 MHz`
- 颜色格式: `RGB565 / 16bit`
- 背光由 `LEDC` PWM 控制，默认亮度为 `70`

说明:
- 例程通过 `esp_lcd` 子系统驱动 LCD。
- `Vernon_ST7789T.c` 是一个板载 ST7789T 的自定义 vendor panel 驱动，里面写死了初始化寄存器序列。

### 4.2 电容触摸 CST328

例程文件:
- `main/Touch_Driver/CST328.h`
- `main/Touch_Driver/CST328.c`
- `main/Touch_Driver/esp_lcd_touch/esp_lcd_touch.*`

接口方式:
- 独立 I2C 总线

引脚映射:

| 功能 | GPIO | 说明 |
| --- | ---: | --- |
| TP_SDA | 1 | 触摸 I2C 数据 |
| TP_SCL | 3 | 触摸 I2C 时钟 |
| TP_INT | 4 | 触摸中断输入 |
| TP_RST | 2 | 触摸复位 |

器件信息:

- I2C 地址: `0x1A`
- I2C 端口号: `I2C_NUM_1`
- I2C 频率: `400 kHz`
- 触摸点数上限: `5`

说明:
- 触摸没有和 RTC/IMU 共用 I2C，而是单独占用一条总线。
- 这是个很重要的板级设计特征，后续你自己扩展 I2C 外设时，要注意不要把触摸和主 I2C 总线混在一起。

### 4.3 主 I2C 总线: RTC + IMU

例程文件:
- `main/I2C_Driver/I2C_Driver.h`
- `main/PCF85063/PCF85063.*`
- `main/QMI8658/QMI8658.*`

主 I2C 引脚:

| 功能 | GPIO | 说明 |
| --- | ---: | --- |
| I2C_SCL | 10 | 主 I2C 时钟 |
| I2C_SDA | 11 | 主 I2C 数据 |

总线参数:

- I2C 端口号: `I2C_NUM_0`
- 频率: `400 kHz`

挂载器件:

| 器件 | 地址 | 说明 |
| --- | ---: | --- |
| PCF85063 | `0x51` | RTC 实时时钟 |
| QMI8658 | `0x6B` | 六轴 IMU，例程使用低地址版本 |

说明:
- RTC 和 IMU 共用一条 I2C 总线。
- 如果你后面还想挂别的 I2C 传感器，优先考虑复用这条总线，而不是触摸那条总线。

### 4.4 RTC: PCF85063

例程文件:
- `main/PCF85063/PCF85063.h`
- `main/PCF85063/PCF85063.c`

连接关系:

- 通过主 I2C 总线接入
- 地址 `0x51`

例程功能:

- 初始化 RTC 控制寄存器
- 轮询读取年月日时分秒
- 支持设置时间/日期/闹钟

### 4.5 IMU: QMI8658

例程文件:
- `main/QMI8658/QMI8658.h`
- `main/QMI8658/QMI8658.c`

连接关系:

- 通过主 I2C 总线接入
- 地址 `0x6B`

例程功能:

- 初始化加速度计/陀螺仪工作模式
- 配置量程、输出数据率、低通滤波
- 周期性读取加速度和角速度

### 4.6 电池电压检测

例程文件:
- `main/BAT_Driver/BAT_Driver.h`
- `main/BAT_Driver/BAT_Driver.c`

连接关系:

| 功能 | GPIO | ADC 通道 | 说明 |
| --- | ---: | --- | --- |
| BAT_ADC | 8 | `ADC1_CH7` | 电池电压采样 |

例程处理逻辑:

- 使用 `adc_oneshot`
- 使用 ADC 校准接口
- 最终换算公式为:

```text
BAT_analogVolts = (voltage_mv * 3.0 / 1000.0) / 0.9945
```

说明:
- 这里的 `3.0` 基本可以理解为板上分压比带来的还原系数。
- 实际精度仍要以原理图分压电阻和实测万用表为准。

### 4.7 电源按键与保持控制

例程文件:
- `main/PWR_Key/PWR_Key.h`
- `main/PWR_Key/PWR_Key.c`

引脚映射:

| 功能 | GPIO | 说明 |
| --- | ---: | --- |
| PWR_KEY_Input | 6 | 电源键输入检测 |
| PWR_Control | 7 | 电源保持/控制输出 |

例程行为:

- 上电时先把 `GPIO7` 拉低
- 检测按键后再拉高保持
- 长按按键会触发不同的电源状态逻辑

说明:
- `Shutdown()` 里会把 `GPIO7` 置低，说明这一路大概率和电源保持电路有关。
- 这是典型的“软开关机”实现方式。

### 4.8 MicroSD 卡槽

例程文件:
- `main/SD_Card/SD_MMC.h`
- `main/SD_Card/SD_MMC.c`

接口方式:
- `SDMMC` 主机
- 例程使用 `1-bit` 模式

引脚映射:

| 功能 | GPIO | 说明 |
| --- | ---: | --- |
| SD_CLK | 14 | SD 时钟 |
| SD_CMD | 17 | SD 命令 |
| SD_D0 | 16 | SD 数据 0 |
| SD_D1 | 未使用 | 例程中设为 `-1` |
| SD_D2 | 未使用 | 例程中设为 `-1` |
| SD_D3 | 未使用 | 例程中设为 `-1` |

补充说明:

- `SD_MMC.h` 里还出现了 `CONFIG_SD_Card_D3 = 21`，但这个值没有真正参与 `SD_Init()` 的 `slot_config`。
- 因此当前官方例程是明确按 `1-bit SDMMC` 跑的，不是 `4-bit`。
- 如果你未来想改成 `4-bit`，要先回到原理图确认 `D1/D2/D3` 是否确实接出、是否和别的板载资源复用。

### 4.9 音频 DAC: PCM5101

例程文件:
- `main/Audio_Driver/PCM5101.h`
- `main/Audio_Driver/PCM5101.c`

接口方式:
- I2S

引脚映射:

| 功能 | GPIO | 说明 |
| --- | ---: | --- |
| I2S_BCLK | 48 | 位时钟 |
| I2S_LRCK / WS | 38 | 左右声道时钟 |
| I2S_DOUT | 47 | 音频数据输出 |
| I2S_MCLK | 未使用 | `GPIO_NUM_NC` |
| I2S_DIN | 未使用 | `GPIO_NUM_NC` |

说明:
- 例程只做音频播放输出，没有做录音输入。
- 音频解码使用 `esp-audio-player + helix mp3`，PCM 数据最终通过 I2S 送到 PCM5101。

### 4.10 Wi-Fi / BLE

例程文件:
- `main/Wireless/Wireless.c`

说明:

- Wi-Fi / BLE 使用 ESP32-S3 芯片内部无线功能，不依赖额外板载外设引脚定义。
- 当前官方例程默认只启用了 Wi-Fi 扫描任务。
- BLE 扫描代码保留在工程里，但创建任务的语句被注释掉了。

## 5. 板级总线拓扑

可以把这块板理解为下面这几条独立总线/功能域:

```text
ESP32-S3
├─ SPI3
│  └─ ST7789T LCD
├─ I2C0
│  ├─ PCF85063 RTC
│  └─ QMI8658 IMU
├─ I2C1
│  └─ CST328 Touch
├─ SDMMC
│  └─ MicroSD (当前例程使用 1-bit)
├─ I2S
│  └─ PCM5101 Audio DAC
├─ ADC1_CH7
│  └─ Battery Voltage Detect
└─ GPIO
   ├─ GPIO5  LCD Backlight PWM
   ├─ GPIO6  Power Key Detect
   └─ GPIO7  Power Hold/Control
```

## 6. 和 COM17 的关系

你现在在 `COM17` 上接入开发板，说明板子已经通过 USB 枚举成功。

对这个工程，后续常用命令可以直接写成:

```powershell
idf.py -p COM17 flash monitor
```

如果只想看串口输出:

```powershell
idf.py -p COM17 monitor
```

## 7. 二次开发时最重要的注意点

- LCD 使用的是 `SPI3`，不要再把同一组 GPIO 直接分配给别的 SPI 外设。
- 触摸使用独立 `I2C1`，不要误以为它和 RTC/IMU 在同一条总线上。
- RTC 和 IMU 共用 `I2C0`，新增 I2C 器件优先接这里。
- SD 卡当前例程是 `1-bit SDMMC`，不是 4-bit。
- 电源按键和 `GPIO7` 电源保持逻辑比较关键，改动前建议先看原理图。
- 音频输出是 I2S 到 PCM5101，若改成别的 DAC，需要同时改 GPIO 和 `audio_player` 的写出链路。

## 8. 本文结论

对于这块 `ESP32-S3-Touch-LCD-2.8`，本地官方例程已经把板载资源划分得很清楚:

- 显示走 `SPI3`
- 触摸走独立 `I2C1`
- RTC/IMU 共用 `I2C0`
- 存储走 `SDMMC`
- 音频走 `I2S`
- 电池检测走 `ADC`
- 电源键通过 `GPIO6/7` 做软开关机

后面你无论是裁剪例程、改 UI、换驱动，还是接你自己的外设，优先遵守这套总线分工，基本就不会踩板级冲突。
