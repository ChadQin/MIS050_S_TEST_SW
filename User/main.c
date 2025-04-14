#include "stm32f10x.h"
#include "sys.h"
#include "stm32f10x_tim.h"
#include "Channel.h"
#include "usart.h"
#include "delay.h"
#include "adc.h"


int main(void) {
	
  RCC_Configuration();
  GPIO_CFG();
	ADC1_Init();
	USART1_Init(921600);
  Channel_1_PWM_Init(359, 1); // 72M / (1+1) / 359+1 = 100KHz PWM 
	PWM_Channel_1_Control(DISABLE);
	printf("初始化完成");
  while (1) {
//		delay_s(6);
//		PWM_Channel_1_Control(DISABLE);
//		delay_s(6);
//		PWM_Channel_1_Control(ENABLE);
//			adc_value = ADC_GetValue(); // 获取ADC值
//      voltage = (adc_value * 3.3f) / 4095; // 转换为电压值（假设VREF=3.3V）
//			printf("Voltage: %.2fV\r\n", voltage);
//			delay_s(1);
		
	}
}
