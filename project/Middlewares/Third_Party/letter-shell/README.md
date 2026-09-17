# letter-shell（vendored / 内置）

- 上游项目：<https://github.com/NevermindZZT/letter-shell>
- 版本：**3.2.3**（见 `shell.h` 中的 `SHELL_VERSION`）
- 来源：本地资料目录 `F411LVGLESP8266资料\letter-shell-shell3.1\letter-shell-shell3.1\src\`
  （上游整套约 0.9 MB，含 `demo/`、`doc/`、`extensions/`，未纳入本仓库）

## 编译进工程的文件

| 文件 | 说明 |
|------|------|
| `shell.c` | 核心：命令解析、执行、导出命令段（`shellCommand`） |
| `shell_ext.c` | 参数自动转换，`shell.c` 中 `shellExtRun()` 依赖它 |

这两个 `.c` 已加入 Keil 工程的 `letter-shell` 分组。

## 保留但未编译的文件

| 文件 | 何时才需要 |
|------|-----------|
| `shell_cmd_list.c` | 关闭 `SHELL_USING_CMD_EXPORT`、改用「命令表模式」时 |
| `shell_companion.c` | 启用伴生对象（`SHELL_USING_COMPANION 1`）时，log / fs 等扩展依赖它 |

## 配置覆盖机制

`shell_cfg.h` 第 15–17 行：

```c
#ifdef SHELL_CFG_USER
#include SHELL_CFG_USER
#endif
```

其中所有配置宏都由 `#ifndef` 保护，因此工程里通过 Keil 的
`Options for Target → C/C++ → Define` 加入

```
SHELL_CFG_USER="shell_cfg_user.h"
```

实际的配置写在 `../../../BSP/Shell/shell_cfg_user.h`，**不改动本目录的上游源码**。

## 维护约定

- 本目录属第三方代码，升级时整体替换，不要在这里直接改配置。
- 用 CubeMX 重新生成代码后，需检查 Keil 工程里 `letter-shell` 分组与包含路径
  是否仍存在 —— CubeMX 会重写 `.uvprojx`，可能冲掉手动添加的条目。
