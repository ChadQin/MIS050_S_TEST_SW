
#include "relay.h"
#include "delay.h"

void RELAY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE); 
	GPIO_InitStructure.GPIO_Pin = TIG_RALAY | MIS_RALAY | PSB_RALAY  | TEST_LED | TEST_LED1; //选择端口号（0~15或all）                        
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //选择IO接口工作方式       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //设置IO接口速度（2/10/50MHz）    
	GPIO_Init(RELAYPORT, &GPIO_InitStructure);	
	GPIO_Init(LEDPORT, &GPIO_InitStructure);	
}

void LED_Blink(void)
{
	for(uint8_t i=0; i<20; i++)
	{
		GPIO_WriteBit(LEDPORT,TEST_LED,(BitAction)(1-(BitAction)GPIO_ReadOutputDataBit(LEDPORT,TEST_LED)));
		delay_ms(40);
	}
}
