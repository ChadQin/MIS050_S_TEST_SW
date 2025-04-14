#include "adc.h"
#include "stm32f10x.h"


/**
  * @brief  初始化ADC1（通道9/PB1）
  * @param  无
  * @retval 无
  */
void ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /* 使能GPIOB和ADC1时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);

    /* 配置PB1为模拟输入模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 模拟输入模式
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* ADC时钟配置（PCLK2的6分频） */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  // 72MHz/6=12MHz

    /* ADC基本参数配置 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;          // 独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                // 单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;          // 单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;       // 数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                      // 1个转换通道
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 配置ADC通道9（PB1对应通道9），采样时间55.5周期 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5);

    /* 使能ADC并校准 */
    ADC_Cmd(ADC1, ENABLE);
    
    /* ADC校准流程 */
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));  // 等待校准复位完成
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));       // 等待校准完成
}

/**
  * @brief  获取ADC转换值
  * @param  无
  * @retval 12位ADC转换结果（0-4095）
  */
uint16_t ADC_GetValue(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);         // 启动转换
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)); // 等待转换完成
    return ADC_GetConversionValue(ADC1);           // 返回转换结果
}
