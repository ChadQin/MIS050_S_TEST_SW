
#include "sys.h"
#include "usart.h"
#include <ctype.h>
#include "Channel.h"
#include "adc.h"
#include "math.h"
#define TIMER_CLOCK       72000000
#define UPDATE_FREQ  0xFFFFFFFF  // 特殊值表示不更新频率
#define UPDATE_DUTY  0xFF        // 特殊值表示不更新占空比

#define RX_BUFFER_SIZE	256


char rx_buffer[RX_BUFFER_SIZE];
int rx_index = 0;
	  	 
//使UASRT串口可用printf函数发送
//在usart.h文件里可更换使用printf函数的串口号	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE {
	int handle; 
}; 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x){ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f){      
	while((USART_n->SR&0X40)==0);//循环发送,直到发送完毕   
    USART_n->DR = (u8) ch;      
	return ch;
}
#endif 


/*
USART1串口相关程序
*/
 
#if EN_USART1   //USART1使用与屏蔽选择
u8 USART1_RX_BUF[USART1_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART1_RX_STA=0;       //接收状态标记	  

/*
USART1专用的printf函数
当同时开启2个以上串口时，printf函数只能用于其中之一，其他串口要自创独立的printf函数
调用方法：USART1_printf("123"); //向USART2发送字符123
*/
void USART1_printf (char *fmt, ...){ 
	char buffer[USART1_REC_LEN+1];  // 数据长度
	u8 i = 0;	
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, USART1_REC_LEN+1, fmt, arg_ptr);
	while ((i < USART1_REC_LEN) && (i < strlen(buffer))){
        USART_SendData(USART1, (u8) buffer[i++]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); 
	}
	va_end(arg_ptr);
}

void USART1_Init(u32 bound){ //串口1初始化并启动
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
     //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
   //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器 
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启ENABLE/关闭DISABLE中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 
}

void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char received_char = USART_ReceiveData(USART1);
        // 将接收到的数据存入缓冲区
        if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = received_char;
        }

        // 如果接收的是换行符或结束符，处理缓冲区中的字符串
        if (received_char == '\n') {
            rx_buffer[rx_index] = '\0';   // 添加字符串结束标志
            process_received_string(rx_buffer);
            rx_index = 0;               // 重置索引
        }
    }
}

/**
 * @brief 处理接收到的字符串函数
 * @param received_str 接收的字符串
 */
void process_received_string(char* received_str) {
    int value1, value2;
		uint16_t adc_value = ADC_GetValue(); 
    float v_adc = (adc_value * 3.3f) / 4095.0f;

    // 同时设置频率和占空比（格式：PWM 5000 75）
    if (sscanf(received_str, "PWM %d %d", &value1, &value2) == 2) {
        set_pwm_params(
            (value1 > 0) ? value1 : UPDATE_FREQ,
            (value2 <= 100) ? value2 : UPDATE_DUTY
        );
        printf("PWM设置：%dHz %d%%\r\n", 
               (value1 > 0) ? value1 : TIMER_CLOCK/((TIM2->ARR+1)*(TIM2->PSC+1)),
               (value2 <= 100) ? value2 : (TIM2->CCR4*100)/(TIM2->ARR+1));
    }
    // 单独设置频率（保留原FREQ命令兼容）
    else if (sscanf(received_str, "FREQ %d", &value1) == 1) {
        set_pwm_params(value1, UPDATE_DUTY);
        printf("频率更新：%dHz\r\n", value1);
    }
    // 单独设置占空比（保留原DUTY命令兼容）
    else if (sscanf(received_str, "DUTY %d", &value1) == 1) {
        set_pwm_params(UPDATE_FREQ, value1);
        printf("占空比更新：%d%%\r\n", value1);
    }
    // 控制命令（保持原逻辑）
    else if (strcmp(received_str, "PWM_ON\r\n") == 0) {
        PWM_Channel_1_Control(ENABLE);
        printf("PWM输出已激活\r\n");
    }
    else if (strcmp(received_str, "PWM_OFF\r\n") == 0) {
        PWM_Channel_1_Control(DISABLE);
        printf("PWM输出已停止\r\n");
    }
		else if (strcmp(received_str, "PWM_QUERY\r\n") == 0) {
        uint32_t current_freq;
        uint8_t current_duty;
        Get_PWM_Parameters(TIM2, &current_freq, &current_duty);
        printf("当前PWM参数：%luHz %d%%\r\n", current_freq, current_duty);
    }
		else if (strcmp(received_str, "TEMP_QUERY\r\n") == 0) {
        if (v_adc >= 3.3f) {
        printf("电压超量程！\r\n");
        return;
			}
				float r_ntc = (10000.0f * v_adc) / (3.3f - v_adc);

				// Steinhart-Hart公式计算温度
				float temp_kelvin = 1.0f / ( (1.0f/298.15f) + (1.0f/3950.0f) * logf(r_ntc/10000.0f) );
				float temp_celsius = temp_kelvin - 273.15f;

				printf("温度：%.1f℃\r\n", temp_celsius);
    }
    else {
        printf(	"无效命令格式！支持格式：\r\n"
								"TEMP_QUERY\t查询温度\r\n"
								"PWM_QUERY\t查询PWM参数\r\n"
								"FREQ 1000\t设置频率(592Hz~36MHz)\r\n"
								"DUTY 50\t\t设置占空比(0-100%%)\r\n"
								"PWM 1000 50\t同时设置\r\n");
    }
}


#endif	

/*
USART2串口相关程序
*/ 
#if EN_USART2   //USART2使用与屏蔽选择
u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART2_RX_STA=0;       //接收状态标记	  

/*
USART2专用的printf函数
当同时开启2个以上串口时，printf函数只能用于其中之一，其他串口要自创独立的printf函数
调用方法：USART2_printf("123"); //向USART2发送字符123
*/
void USART2_printf (char *fmt, ...){ 
	char buffer[USART2_REC_LEN+1];  // 数据长度
	u8 i = 0;	
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, USART2_REC_LEN+1, fmt, arg_ptr);
	while ((i < USART2_REC_LEN) && (i < strlen(buffer))){
        USART_SendData(USART2, (u8) buffer[i++]);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); 
	}
	va_end(arg_ptr);
}


void USART2_Init(u32 bound){ //串口1初始化并启动
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
		 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE); //使能UART2所在GPIOA的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //使能串口的RCC时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //设置USART2的RX接口是PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //设置USART2的TX接口是PA2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

   //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启ENABLE/关闭DISABLE中断
    USART_Cmd(USART2, ENABLE);                    //使能串口 
   //Usart2 NVIC 配置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器 
}

void USART2_IRQHandler(void){ //串口2中断服务程序（固定的函数名不能修改）	

} 
#endif	


#if EN_USART3   //如果使能了接收
u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART3_RX_STA=0;       //接收状态标记	  

/*
USART3专用的printf函数
当同时开启2个以上串口时，printf函数只能用于其中之一，其他串口要自创独立的printf函数
调用方法：USART3_printf("123"); //向USART3发送字符123
*/
void USART3_printf (char *fmt, ...){ 
	char buffer[USART3_REC_LEN+1];  // 数据长度
	u8 i = 0;	
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, USART3_REC_LEN+1, fmt, arg_ptr);
	while ((i < USART3_REC_LEN) && (i < strlen(buffer))){
        USART_SendData(USART3, (u8) buffer[i++]);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); 
	}
	va_end(arg_ptr);
}

void USART3_Init(u32 BaudRate){ //USART3初始化并启动
   GPIO_InitTypeDef GPIO_InitStructure;
   USART_InitTypeDef USART_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure; 

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE); //使能UART3所在GPIOB的时钟
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能串口的RCC时钟

   //串口使用的GPIO口配置
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//设置USART3的RX接口是PB11
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//接口模式 浮空输入
   GPIO_Init(GPIOB, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//设置USART3的TX接口是PB10
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//输出速度50MHz
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//接口模式 复用推挽输出
   GPIO_Init(GPIOB, &GPIO_InitStructure);

   //配置串口
   USART_InitStructure.USART_BaudRate = BaudRate;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
   USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
   USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

   USART_Init(USART3, &USART_InitStructure);//配置串口3
   USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//使能串口接收中断  
   //USART_ITConfig(USART3, USART_IT_TXE, ENABLE);//串口发送中断在发送数据时开启
   USART_Cmd(USART3, ENABLE);//使能串口3

   //串口中断配置
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//允许USART3中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//中断等级
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

//串口3中断服务程序（固定的函数名不能修改）
void USART3_IRQHandler(void){ 	

}

#endif	
