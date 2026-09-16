# F411LVGLESP8266

STM32F411CEU6（WeAct 黑药丸）+ ESP8266 + 1.69″ ST7789(CTP) 温湿度小仪表。

完整硬件方案、引脚表、总线策略、开发节奏见 **[PROJECT_SETUP.md](PROJECT_SETUP.md)**（本仓库的权威备忘）。

---

## 仓库内容

| 路径 | 说明 |
|------|------|
| `PROJECT_SETUP.md` | 方案备忘：硬件清单、完整引脚表、软件组件、开发节奏、易错点 |
| `project/` | **主力工程**：CubeMX + FreeRTOS + `BSP/AHT21`、`BSP/ST7789`（引脚已对齐备忘） |

## 已排除在版本管理之外（见 `.gitignore`）

以下为**参考资料 / 上游 SDK / 中间件 / 压缩包**，体积大或属第三方代码，未入库。
它们已从仓库目录移至同级 `E:\Workspace\F411LVGLESP8266资料\`：

- `letter-shell-shell3.1/`（上游 letter-shell 3.1 调试中间件，规划接入 USART1）
- `ST7789/`（1.4 GB，含 3 份重复的 ST7789 参考例程）
- `WeActStudio.MiniSTM32F4x1-master/`（上游 SDK，318 MB）
- `AHT21_Working_Original_Hardware_Testing/`（AHT21 参考验证工程，163 MB）
- `3526526985P169H002-CTP资料 (1)/`（屏厂资料，367 MB）
- 所有 `*.zip` / `*.7z` / `*.rar` 压缩包
- Keil 编译产物（`MDK-ARM/*/` 输出目录、`*.o` / `*.crf` / `*.axf` / `*.map` 等）
- Keil 个人界面配置（`*.uvguix.*`）

需要把其中某个目录纳入版本管理时，删掉 `.gitignore` 第 1 节里对应的一行即可。

## 开发环境

- Keil MDK-ARM（工程文件为 `MDK-ARM/*.uvprojx`，`MDK-ARM/RTE` 已保留）
- STM32CubeMX（`*.ioc`）用于改引脚 / 外设后重新生成代码
- 调试串口：USART1（PA9/PA10）↔ USB-TTL，接 letter-shell

> 注：本机未安装 ARM 工具链（`arm-none-eabi-gcc` / `make` / `cmake`）与 Keil MDK，仓库内未提供可交叉编译验证的命令行构建；编译在装有 MDK 的机器上进行。

## 计划

一期本地仪表（LVGL 温湿度界面 + letter-shell）→ 二期 ESP8266 上报 → 三期 ESP 代理无线 OTA（见 `PROJECT_SETUP.md` 第 2、7 节）。
