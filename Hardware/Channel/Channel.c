#include <stddef.h>  // 确保NULL定义可用
#include "Channel.h"
#include "delay.h"
#include <math.h>
#include <stdint.h>

#define TIMER_CLOCK       72000000
#define UPDATE_FREQ  0xFFFFFFFF  // 特殊值表示不更新频率
#define UPDATE_DUTY  0xFF        // 特殊值表示不更新占空比

void GPIO_CFG() {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  
  // 关键修正：仅启用TIM2完全重映射
  GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Channel_1_PWM_Init(uint16_t period, uint16_t prescaler) {
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = (period + 1) / 2; // 50%占空比
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 修正极性
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);

  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
  TIM_Cmd(TIM2, ENABLE);
}


/**
  * @brief  启用或禁用TIM2通道1的PWM输出
  * @param  state: ENABLE 或 DISABLE
  * @retval 无
  */
void PWM_Channel_1_Control(FunctionalState state)
{
    // 控制通道输出使能
    TIM_CCxCmd(TIM2, TIM_Channel_4, state);
    
    // 可选：同时控制定时器启停（根据需求）
    // TIM_Cmd(TIM2, state);
}

/**
* @brief 综合PWM参数设置函数（原子操作）
* @param freq_hz 目标频率（0xFFFFFFFF表示不更新）
* @param duty_percent 目标占空比（0xFF表示不更新）
*/
void set_pwm_params(uint32_t freq_hz, uint8_t duty_percent) {
    // 获取当前配置
    uint16_t current_psc = TIM2->PSC;
    uint32_t actual_clock = TIMER_CLOCK / (current_psc + 1);
    uint32_t new_arr = TIM2->ARR;
    uint32_t new_ccr = TIM2->CCR4;

    // 处理频率更新
    if (freq_hz != UPDATE_FREQ) {
        // 计算ARR时考虑最小频率容错
        uint32_t max_arr = 0xFFFF;
        // 修改代码
				new_arr = (actual_clock / fmaxf(freq_hz, 1)) - 1;  // 替换 __max
				new_arr = fminf(new_arr, max_arr);                 // 替换 __min

        // 自动保持当前占空比比例
        uint32_t current_duty = (new_ccr * 100) / (TIM2->ARR + 1);
        // 修改代码
				current_duty = fminf(current_duty, 100);            // ?:ml-citation{ref="5,7" data="citationList"}
				new_ccr = (new_arr + 1) * fminf(duty_percent, 100) / 100;
    }

    // 处理占空比更新
    if (duty_percent != UPDATE_DUTY) {
        new_ccr = (new_arr + 1) * (uint32_t)fminf(duty_percent, 100) / 100;  // ?:ml-citation{ref="5,6" data="citationList"}

    }

    // 临界区操作
    TIM2->CR1 &= ~TIM_CR1_CEN;        // 暂停定时器
    if (freq_hz != UPDATE_FREQ) {
        TIM2->ARR = new_arr;          // 更新周期值
    }
    TIM2->CCR4 = new_ccr;             // 更新占空比值
    TIM2->EGR |= TIM_EGR_UG;          // 强制更新寄存器
    TIM2->CR1 |= TIM_CR1_CEN;         // 恢复定时器
}

/**
  * @brief  获取当前PWM参数
  * @param  TIMx: 定时器实例（如TIM2）
  * @param  freq: 返回频率值（单位Hz）
  * @param  duty: 返回占空比（单位%，0-100）
  * @retval 无
  */
void Get_PWM_Parameters(TIM_TypeDef* TIMx, uint32_t *freq, uint8_t *duty) {
    if (TIMx == NULL || freq == NULL || duty == NULL) return;

    // 计算当前频率（需根据实际定时器时钟配置修改TIMER_CLOCK）
    uint32_t timer_clock = TIMER_CLOCK; // 示例：72MHz
    uint32_t prescaler = TIMx->PSC + 1;
    uint32_t autoreload = TIMx->ARR + 1;
    *freq = timer_clock / (prescaler * autoreload);

    // 计算当前占空比（假设使用通道4，即CCR4）
    *duty = (TIMx->CCR4 * 100) / autoreload;
}

