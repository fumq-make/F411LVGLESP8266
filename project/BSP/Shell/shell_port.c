/**
  * @file    shell_port.c
  * @brief   letter-shell 移植层：USART1 <-> shell，基于 FreeRTOS Stream Buffer
  *
  * 数据流：
  *   PC 串口 -> USART1 RX 中断（单字节）-> StreamBuffer -> shellTask -> 命令执行
  *   命令输出 -> shell.write -> HAL_UART_Transmit -> USART1 TX -> PC 串口
  *
  * 依赖：
  *   - CubeMX 中已使能 USART1（Asynchronous, 115200-8-N-1），且
  *     USART1 global interrupt 抢占优先级设为 >= 5
  *     （FreeRTOSConfig.h: configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5，
  *      优先级数值小于 5 的中断里不允许调用 FreeRTOS 的 FromISR API）
  *   - FreeRTOS stream_buffer.c 已在 Keil 工程中（本工程已包含）
  */

#include "shell_port.h"

#include "main.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "cmsis_os.h"

/* ------------------------------------------------------------- 对象定义 */
Shell shell;                                        /**< letter-shell 对象    */
static char shellBuffer[512];                       /**< shell 命令行缓冲      */
static StreamBufferHandle_t shellRxStream = NULL;   /**< 串口 -> shell 字节流  */
static uint8_t shellRxByte;                         /**< 中断接收单字节暂存    */

/* ------------------------------------------------------------- 读写接口 */
/* 原型见 shell.h: signed short (*read / *write)(char *, unsigned short) */

/**
  * @brief  shell 写：阻塞发送到 USART1
  */
static signed short userShellWrite(char *data, unsigned short len)
{
    if (HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 0xFFFF) != HAL_OK)
    {
        return 0;
    }
    return (signed short)len;
}

/**
  * @brief  shell 读：从 Stream Buffer 取数据
  * @note   取不到数据时阻塞 100ms 后返回 0，让出 CPU 给其它任务；
  *         shellTask 收到长度为 0 时会继续循环重试。
  */
static signed short userShellRead(char *data, unsigned short len)
{
    size_t received = xStreamBufferReceive(shellRxStream, data, len,
                                           pdMS_TO_TICKS(100));

    return (signed short)received;
}

/* --------------------------------------------------------------- 初始化 */
static const osThreadAttr_t shellTask_attributes = {
    .name = "shellTask",
    .stack_size = 1024,                     /**< CMSIS-RTOS2 单位是「字节」 */
    .priority = (osPriority_t)osPriorityNormal,
};

void userShellInit(void)
{
    /* 1. 建立串口字节流缓冲：容量 256 B，触发阈值 1 B（来一个字节即唤醒） */
    shellRxStream = xStreamBufferCreate(256, 1);
    configASSERT(shellRxStream != NULL);

    /* 2. 打开 USART1 单字节中断接收 */
    HAL_UART_Receive_IT(&huart1, &shellRxByte, 1);

    /* 3. 绑定读写接口并初始化 shell */
    shell.write = userShellWrite;
    shell.read  = userShellRead;
    shellInit(&shell, shellBuffer, (unsigned short)sizeof(shellBuffer));

    /* 4. 建立 shell 任务（shellTask 由 letter-shell 提供，内部为 while(1) 循环） */
    if (osThreadNew(shellTask, &shell, &shellTask_attributes) == NULL)
    {
        Error_Handler();
    }
}

/* --------------------------------------------------------- 串口中断回调 */
/**
  * @brief  USART1 接收完成回调（HAL 弱函数，此处实现）
  * @note   该中断优先级必须 >= 5，才能安全调用 FreeRTOS 的 FromISR API
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xStreamBufferSendFromISR(shellRxStream, &shellRxByte, 1,
                                 &xHigherPriorityTaskWoken);

        /* 重新武装，等待下一个字节 */
        HAL_UART_Receive_IT(&huart1, &shellRxByte, 1);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* --------------------------------------------------------- D1 自测命令 */
/**
  * @brief  串口输入 hello 时的测试命令，用于验证 shell 通路与命令导出段
  */
static int shellHello(void)
{
    shellPrint(shellGetCurrent(), "hello letter-shell, tick = %u\r\n",
               (unsigned int)HAL_GetTick());
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)
                 | SHELL_CMD_DISABLE_RETURN,
                 hello, shellHello, print hello);
