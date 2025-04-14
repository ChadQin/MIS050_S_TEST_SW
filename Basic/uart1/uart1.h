#ifndef __UART1_H
#define __UART1_H

#include "stm32f10x.h"    // 根据具体芯片型号替换头文件，如 STM32F4 则为 stm32f4xx.h

// 初始化 UART1
void uart1_init(uint32_t baud_rate);

// 发送字符串
void uart1_send_string(const uint8_t *str);

// 接收字符串处理函数
void uart1_receive_process(void);
void process_received_command(uint8_t *command);

#endif /* __UART1_H */
