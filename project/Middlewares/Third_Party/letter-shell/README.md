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

## 本地补丁：配置包含方式（**重要**）

上游 `shell_cfg.h` 的写法是靠编译选项传入头文件名：

```c
#ifdef SHELL_CFG_USER
#include SHELL_CFG_USER
#endif
```

但 Keil MDK 对这个字符串宏的处理不可靠 —— 引号会被剥掉，变成
`#include shell_cfg_user.h`，报 `error: #13: expected a file name`。
实测两种填法均失败：

| 填法 | 结果 |
|------|------|
| `C/C++ → Define` 框：`SHELL_CFG_USER="shell_cfg_user.h"` | ❌ 引号被吞 |
| `C/C++ → Misc Controls`：`-DSHELL_CFG_USER="shell_cfg_user.h"` | ❌ 引号仍被吞 |

因此本目录的 `shell_cfg.h` 已打**本地补丁**，把上面 3 行换成直接包含：

```c
#include "shell_cfg_user.h"
```

用户配置实际写在 `../../../BSP/Shell/shell_cfg_user.h`（其中所有宏都被上游的
`#ifndef` 保护，可以正常覆盖默认值）。

**因此 Keil 里不需要配置任何 `SHELL_CFG_USER` 宏**；若 Define / Misc Controls
中还留着它，建议删除以免混淆（留着也不会报错，该宏已无人使用）。

## 维护约定

- 本目录属第三方代码，升级时整体替换；**替换后必须重新打上述补丁**，
  否则会退回依赖编译开关的状态而编译失败。
- 除该补丁外，不要在这里改配置 —— 配置一律写在 `BSP/Shell/shell_cfg_user.h`。
- 用 CubeMX 重新生成代码后，需检查 Keil 工程里 `letter-shell` 分组与包含路径
  是否仍存在 —— CubeMX 会重写 `.uvprojx`，可能冲掉手动添加的条目。
