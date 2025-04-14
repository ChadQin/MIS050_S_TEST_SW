#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

// 函数声明（禁止包含函数实现！）
void TIM3_PWM_Init(u16 arr, u16 psc);
void TIM3_SetDutyCycle(u16 compare);

#endif // __PWM_H
