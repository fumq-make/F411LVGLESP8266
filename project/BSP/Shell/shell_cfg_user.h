/**
  * @file    shell_cfg_user.h
  * @brief   letter-shell 用户配置（D1：USART1 + letter-shell 打通）
  *
  * 生效方式：Keil → Options for Target → C/C++ → Define 里加
  *     SHELL_CFG_USER="shell_cfg_user.h"
  * shell_cfg.h 第 15-17 行会据此 #include 本文件；其中所有宏均为
  * #ifndef 保护，因此这里定义的会覆盖上游默认值。
  */

#ifndef __SHELL_CFG_USER_H__
#define __SHELL_CFG_USER_H__

#include "stm32f4xx_hal.h"

/* D1 不需要伴生对象（log/fs 等扩展才用），省掉 SHELL_MALLOC/SHELL_FREE 依赖 */
#define     SHELL_USING_COMPANION       0

/* 目前只有 shellTask 一个写者，暂不开锁；将来多任务共用时再打开并实现 lock/unlock */
#define     SHELL_USING_LOCK            0

/* 回车触发：CR 与 LF 都接受，兼容各种串口助手；
   注意 SHELL_ENTER_CRLF 不能与这两个同时打开 */
#define     SHELL_ENTER_LF              1
#define     SHELL_ENTER_CR              1
#define     SHELL_ENTER_CRLF            0

/* 双击 tab 补全、超时锁定依赖它 */
#define     SHELL_GET_TICK()            HAL_GetTick()

/* 登录时不发 ANSI 清屏转义序列，避免部分串口助手显示成乱码 */
#define     SHELL_CLS_WHEN_LOGIN        0

#define     SHELL_SHOW_INFO             1
#define     SHELL_DEFAULT_USER          "letter"
#define     SHELL_DEFAULT_USER_PASSWORD ""
#define     SHELL_HISTORY_MAX_NUMBER    5

#endif /* __SHELL_CFG_USER_H__ */
