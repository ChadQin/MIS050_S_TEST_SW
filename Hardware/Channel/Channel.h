#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "stm32f10x.h"

void GPIO_CFG(void);
void Channel_1_PWM_Init(uint16_t period, uint16_t prescaler);
void PWM_Channel_1_Control(FunctionalState state);
void set_pwm_params(uint32_t freq_hz, uint8_t duty_percent);
void Get_PWM_Parameters(TIM_TypeDef* TIMx, uint32_t *freq, uint8_t *duty);

#endif /* __CHANNEL_H */
