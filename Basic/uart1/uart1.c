#include "uart1.h"
#include <stdio.h>
#include <string.h>

#define UART1_BUFFER_SIZE    256   // 接收缓冲区大小

static uint8_t rx_buffer[UART1_BUFFER_SIZE];  // 接收缓冲区
static uint16_t rx_index = 0;                  // 接收索引

/**
 * @brief 初始化 UART1，配置波特率、数据位、停止位等参数。
 * @param baud_rate 波特率（如 9600、115200 等）
 */
void uart1_init(uint32_t baud_rate)
{
    GPIO_InitTypeDef gpioInitStruct;
    USART_InitTypeDef uartInitStruct;

    // 使能 GPIOA 和 UART1 的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // 配置 UART1 的 TX 和 RX 引脚（PA9 = TX，PA10 = RX）
    gpioInitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_OD; // 复用开漏模式
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInitStruct);

    // 配置 UART1 参数
    uartInitStruct.USART_BaudRate = baud_rate;
    uartInitStruct.USART_WordLength = USART_WordLength_8b;  // 8 数据位
    uartInitStruct.USART_StopBits = USART_StopBits_1;        // 1 停止位
    uartInitStruct.USART_Parity = USART_Parity_No;           // 无校验
    uartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uartInitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &uartInitStruct);

    // 使能 UART1 的接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 配置 NVIC 中断优先级（假设使用 STM32F1）
    NVIC_InitTypeDef nvicInitStruct;
    nvicInitStruct.NVIC_IRQChannel = USART1_IRQn;
    nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x00; // 中断优先级
    nvicInitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_Init(&nvicInitStruct);

    // 使能 UART1
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief 发送字符串
 * @param str 待发送的字符串
 */
void uart1_send_string(const uint8_t *str)
{
    while (*str != '\0')
    {
        // 等待发送寄存器为空
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
        // 发送一个字符
        USART_SendData(USART1, *str);
        str++;
    }
}

/**
 * @brief 处理接收到的数据
 */
void uart1_receive_process(void)
{
    static uint8_t received_char = 0;

    // 检查接收数据寄存器是否非空
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
        // 读取接收到的数据
        received_char = USART_ReceiveData(USART1);

        // 将字符添加到缓冲区
        if (rx_index < UART1_BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index++] = received_char;
        }
        else
        {
            // 缓冲区满，清空并重置索引
            rx_index = 0;
        }

        // 检查是否接收到换行符（表示一条完整命令）
        if (received_char == '\n')
        {
            // 处理接收到的字符串
            rx_buffer[rx_index] = '\0';  // 添加字符串结束标志
            process_received_command(rx_buffer);
            rx_index = 0;               // 重置索引
        }
    }
}

/**
 * @brief 处理接收到的命令
 * @param command 接收的字符串
 */
void process_received_command(uint8_t *command)
{
    // 示例：判断接收到的命令并执行相应操作
    if (strcmp((char *)command, "LED_ON") == 0)
    {
        // 执行 LED 点亮操作
        printf("LED ON\r\n");
        uart1_send_string("LED is on!\r\n");
    }
    else if (strcmp((char *)command, "LED_OFF") == 0)
    {
        // 执行 LED 熄灭操作
        printf("LED OFF\r\n");
        uart1_send_string("LED is off!\r\n");
    }
    else
    {
        // 不识别的命令
        uart1_send_string("Unknown command!\r\n");
    }
}

/**
 * @brief UART1 中断服务函数（IRQ Handler）
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        // 处理接收到的数据
        uart1_receive_process();
    }
}
