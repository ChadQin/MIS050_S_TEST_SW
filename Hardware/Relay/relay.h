#ifndef __RELAY_H
#define __RELAY_H	 
#include "sys.h"

#define RELAYPORT GPIOB
#define LEDPORT	GPIOC
#define TIG_RALAY	GPIO_Pin_5
#define MIS_RALAY	GPIO_Pin_6
#define PSB_RALAY	GPIO_Pin_7
#define	TEST_LED	GPIO_Pin_13
#define TEST_LED1	GPIO_Pin_10


void RELAY_Init(void);
void LED_Blink(void);
#endif
