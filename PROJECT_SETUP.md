# STM32F411 + ESP8266 温湿度小仪表 — 方案备忘

> 供下次继续开发时直接引用本文件。  
> 更新日期：2026-09-10  
> 目标：屏上显示温湿度，可选 Wi‑Fi 上传；后续可用 ESP8266 做无线升级代理。

---

## 1. 硬件清单

| 模块 | 型号/说明 | 角色 |
|------|-----------|------|
| 主控 | **STM32F411CEU6**（WeAct 黑药丸） | 屏、传感器、UI、IAP |
| Wi‑Fi | **ESP8266** | 上报数据；后续 OTA 代理（二期） |
| 显示+触摸 | **P169H002-V5-CTP**（1.69″，240×280，ST7789T3，带 CTP） | 屏走 SPI；触摸走 I²C |
| 温湿度 | **AHT21** | I²C，地址通常 `0x38` |
| EEPROM | **AT24C02**（256 字节） | I²C，地址通常 `0x50`（A0/A1/A2=GND） |
| Flash | **W25Q64** | SPI，固件缓存/资源（可后接） |
| 调试 | **USB‑TTL** | letter-shell 接电脑 |

屏转接板：FPC-0.5-18P → 2.54 排针（丝印含屏 SPI + 触摸 I²C）。

---

## 2. 架构原则

```
应用层：温湿度仪表、页面、报警、上报
    ├── LVGL（UI 中间件）
    └── letter-shell（调试控制台中间件）
驱动层：ST7789、CTP、AHT21、AT24C02、W25Q64、UART、ESP
HAL / CubeMX
```

- **LVGL** 与 **letter-shell** 平级中间件，通过同一套驱动访问硬件。  
- **一期**：本地仪表 + shell；**二期**：ESP 上报；**三期**：无线 Bootloader/OTA。  
- ESP 做 OTA 时：推荐「ESP 下载 → 落入 W25Q64 → STM32 自定义 IAP」，不要一上来走 BOOT0 系统 Bootloader。  
- AT24C02 只存参数；大镜像/字库用 W25Q64。

---

## 3. 总线策略

| 总线 | 类型 | 成员 | 区分方式 |
|------|------|------|----------|
| I²C1 | **硬件 I²C**（B6/B7） | 触摸 CTP + AHT21 + AT24C02 | 从机地址 |
| SPI1 | **硬件 SPI**（可 + DMA 刷屏） | 屏 ST7789 + W25Q64 | 独立 CS |
| USART1 | 硬件串口 | USB‑TTL ↔ letter-shell | — |
| USART2 | 硬件串口 | ESP8266（预留） | — |

说明：

- 转接板 **SDA = 屏 MOSI**；**TSDA = 触摸 I²C**，勿混。  
- 屏与 Flash 共用 SCK/MOSI；Flash 单独接 MISO；**同一时刻只拉低一个 CS**。  
- I²C 用硬件，不用软件 I²C（除非排障）。  
- 刷屏建议最终 **SPI1 TX + DMA**；Flash 短命令可先轮询。  
- 电脑调试选 **USB‑TTL**（不用板载 USB CDC）。

---

## 4. 完整引脚表（黑药丸 WeAct F411）

### 4.1 SPI：屏 + W25Q64

| 功能 | STM32 | 对接 |
|------|-------|------|
| SCK | **PA5** | 屏 SCK + W25Q64 CLK |
| MISO | **PA6** | 仅 W25Q64 DO |
| MOSI | **PA7** | 屏 SDA + W25Q64 DI |
| 屏 CS | **PA4** | 转接板 CS |
| Flash CS | **PB0** | W25Q64 CS |
| 屏 DC | **PB1** | 转接板 DC |
| 屏 RST | **PA1** | 转接板 RST |
| 屏背光 | **PA0** | 转接板 PWR（也可直连 3V3） |

W25Q64：WP、HOLD 通常拉到 3V3。

### 4.2 I²C：触摸 + AHT21 + AT24C02

| 功能 | STM32 | 对接 |
|------|-------|------|
| SCL | **PB6** | TSCL + AHT21 SCL + AT24 SCL |
| SDA | **PB7** | TSDA + AHT21 SDA + AT24 SDA |
| 触摸 INT | **PB8** | TINT |
| 触摸 RST | **PB9** | TRST |

- SCL/SDA 上拉到 3.3V（模块已带则可不再堆很多电阻）。  
- AT24C02：A0/A1/A2→GND → 地址 **0x50**；WP→GND 允许写。  
- 转接板 **BUSY、TMISO** 先悬空。

### 4.3 串口

| 功能 | STM32 | 对接 |
|------|-------|------|
| Shell TX | **PA9** (USART1_TX) | USB‑TTL **RX** |
| Shell RX | **PA10** (USART1_RX) | USB‑TTL **TX** |
| ESP TX | **PA2** (USART2_TX) | ESP8266 RX |
| ESP RX | **PA3** (USART2_RX) | ESP8266 TX |
| ESP RST（可选） | **PA8** | ESP RST/EN |

USB‑TTL：GND 共地；勿用 5V TTL 直灌；板子供电用自身 USB/5V，TTL 的 5V 先不接。

### 4.4 建议勿占用

- **PC13**：板载 LED  
- **NRST / SWD（SCK、DIO）**：复位与调试  
- **PA11/PA12**：USB（本方案 shell 不用 CDC，可留空）

### 4.5 电源

- 全系统逻辑电 **3.3V**；屏 VCC 接 3V3，**不要 5V**。  
- ESP8266 峰值电流大，供电要足，与 STM32 **共地**。

---

## 5. 屏转接板丝印对照

| 丝印 | 含义 | 接法 |
|------|------|------|
| VCC / GND | 电源 | 3V3 / G |
| SCK / SDA / CS / DC / RST / PWR | 屏 SPI + 背光 | → SPI 表 |
| TSCL / TSDA / TINT / TRST | 触摸 I²C | → I²C 表 |
| BUSY / TMISO | 可选 | 悬空 |

---

## 6. 软件组件

| 组件 | 用途 |
|------|------|
| STM32CubeMX + HAL | 时钟、SPI1、I2C1、USART1/2、可选 DMA |
| **letter-shell** | 调试命令（temp、i2cscan、eeprom…） |
| **LVGL** | 温湿度 UI、触控 |
| AHT21 / AT24 / ST7789 / CTP / W25 驱动 | 移植层 |
| ESP AT 或自定义协议 | 二期上报 / 三期 OTA |

建议 shell 命令示例：`temp`、`i2cscan`、`eeprom r/w`、`reboot`。

---

## 7. 开发节奏（每天约 5h，有 CubeMX 经验）

一期本地仪表约 **10～15 天（2～3 周）**：

| 天 | 验收目标 |
|----|----------|
| D1 | USART1 + letter-shell 通 |
| D2 | ST7789 硬件 SPI 色块 |
| D3 | AHT21 读数 + shell `temp` |
| D4 | AT24C02 + `i2cscan`（见 0x38、0x50） |
| D5 | 触摸坐标/按下 |
| D6～D7 | LVGL 移植 + 温湿度界面 |
| D8 | 触摸进 LVGL indev |
| D9 | EEPROM 存参 + 界面整理 |
| D10 | 稳定可演示 |
| D11～D12 | 可选 SPI DMA、W25Q64 |
| 之后 | ESP 上报 → OTA |

检查点：第 3 天必须「能出温湿度」；第 10 天「LVGL + shell 可用」。

---

## 8. 通电/接线顺序

1. 电源共地 → USB‑TTL + shell  
2. 屏 SPI（含 PWR）→ 色块  
3. 触摸 I²C → 扫地址  
4. 并上 AHT21、AT24C02  
5. W25Q64  
6. 最后 ESP8266  

---

## 9. 易错点

1. 屏 **SDA** 与触摸 **TSDA** 搞混  
2. USB‑TTL TX/RX 未交叉（MCU PA9→TTL RX，PA10←TTL TX）  
3. 屏接 5V  
4. SPI 双设备同时 CS 有效  
5. I²C 无上拉或 AT24 A0～A2 悬空  
6. 屏 DMA 未完成时访问 W25Q64  

---

## 10. 下次提问时可直接说

> 按 `E:\Workspace\F411LVGLESP8266\PROJECT_SETUP.md` 继续，从 D× / 某某驱动开始。

本目录现有 `project`（主力工程）与 `letter-shell-shell3.1`（调试中间件），新工程建议与备忘引脚表对齐，避免旧工程引脚冲突。
