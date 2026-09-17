/**
  * @file    shell_port.h
  * @brief   letter-shell 在 STM32F411 + FreeRTOS 上的移植接口
  */

#ifndef __SHELL_PORT_H__
#define __SHELL_PORT_H__

#include "shell.h"

/** letter-shell 全局对象，命令函数里可用 shellGetCurrent() 取当前 shell */
extern Shell shell;

/**
  * @brief  初始化 USART1 接收、绑定 shell 读写接口并创建 shellTask
  * @note   必须在 osKernelInitialize() 之后、osKernelStart() 之前调用，
  *         本工程在 freertos.c 的 MX_FREERTOS_Init() 中挂接。
  */
void userShellInit(void);

#endif /* __SHELL_PORT_H__ */
