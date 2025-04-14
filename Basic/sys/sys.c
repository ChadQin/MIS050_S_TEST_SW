

/*
//杜洋工作室出品
//洋桃系列开发板应用程序
//关注微信公众号：洋桃电子
//洋桃开发板资料下载 www.DoYoung.net/YT 
//即可免费看所有教学视频，下载技术资料，技术疑难提问
//更多内容尽在 杜洋工作室主页 www.doyoung.net
*/

/*
《修改日志》
1-201708221422 加入RCC时钟的设置函数。


*/


#include "sys.h"

void NVIC_Configuration(void){ //嵌套中断向量控制器 的设置
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
}

void RCC_Configuration(void) {
  RCC_DeInit();
  RCC_HSEConfig(RCC_HSE_ON);
  if (RCC_WaitForHSEStartUp() == SUCCESS) {
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08);
  }
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

/*********************************************************************************************
 外部8MHz晶振 
*********************************************************************************************/

//void RCC_Configuration(void){ //RCC时钟的设置  
//	ErrorStatus HSEStartUpStatus;   
//	RCC_DeInit();              /* RCC system reset(for debug purpose) RCC寄存器恢复初始化值*/   
//	RCC_HSEConfig(RCC_HSE_ON); /* Enable HSE 使能外部高速晶振*/   
//	HSEStartUpStatus = RCC_WaitForHSEStartUp(); /* Wait till HSE is ready 等待外部高速晶振使能完成*/   
//	if(HSEStartUpStatus == SUCCESS){   
//		/*设置PLL时钟源及倍频系数*/   
//		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //RCC_PLLMul_x（枚举2~16）是倍频值。当HSE=8MHZ,RCC_PLLMul_9时PLLCLK=72MHZ   
//		/*设置AHB时钟（HCLK）*/   
//		RCC_HCLKConfig(RCC_SYSCLK_Div1); //RCC_SYSCLK_Div1——AHB时钟 = 系统时钟(SYSCLK) = 72MHZ（外部晶振8HMZ）   
//		/*注意此处的设置，如果使用SYSTICK做延时程序，此时SYSTICK(Cortex System timer)=HCLK/8=9MHZ*/   
//		RCC_PCLK1Config(RCC_HCLK_Div2); //设置低速AHB时钟（PCLK1）,RCC_HCLK_Div2——APB1时钟 = HCLK/2 = 36MHZ（外部晶振8HMZ）   
//		RCC_PCLK2Config(RCC_HCLK_Div1); //设置高速AHB时钟（PCLK2）,RCC_HCLK_Div1——APB2时钟 = HCLK = 72MHZ（外部晶振8HMZ）   
//		/*注：AHB主要负责外部存储器时钟。APB2负责AD，I/O，高级TIM，串口1。APB1负责DA，USB，SPI，I2C，CAN，串口2，3，4，5，普通TIM */  
//		FLASH_SetLatency(FLASH_Latency_2); //设置FLASH存储器延时时钟周期数   
//		/*FLASH时序延迟几个周期，等待总线同步操作。   
//		推荐按照单片机系统运行频率：
//		0—24MHz时，取Latency_0；   
//		24—48MHz时，取Latency_1；   
//		48~72MHz时，取Latency_2*/   
//		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //选择FLASH预取指缓存的模式，预取指缓存使能   
//		RCC_PLLCmd(ENABLE);	//使能PLL
//		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //等待PLL输出稳定   
//		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //选择SYSCLK时钟源为PLL
//		while(RCC_GetSYSCLKSource() != 0x08); //等待PLL成为SYSCLK时钟源   
//	}  
//	/*开始使能程序中需要使用的外设时钟*/   
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |   
////	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE); //APB2外设时钟使能      
////	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //APB1外设时钟使能  
////	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);   
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);   	 
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    
//}  

/*********************************************************************************************
 内部8MHz晶振
*********************************************************************************************/

//void RCC_Configuration(void){ //RCC时钟的设置  
//    ErrorStatus HSIStartUpStatus;   
//    RCC_DeInit();              /* RCC寄存器恢复初始化值*/   
//    
//    // 启用内部高速时钟(HSI，典型8MHz)
//    RCC_HSICmd(ENABLE);                  // 使能HSI
//    HSIStartUpStatus = RCC_WaitForHSIStartUp(); // 等待HSI稳定
//    if(HSIStartUpStatus != SUCCESS){     
//        // 处理HSI启动失败情况（根据需求添加）
//    }

//    /*设置PLL时钟源及倍频系数（HSI=8MHz，倍频9倍到72MHz）*/   
//    RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_9);  // 修正此处！HSI无需分频参数
//    
//    /*设置AHB时钟（HCLK）*/   
//    RCC_HCLKConfig(RCC_SYSCLK_Div1); // AHB时钟 = 系统时钟(SYSCLK) = 72MHz
//    
//    /*设置低速APB时钟（PCLK1）*/   
//    RCC_PCLK1Config(RCC_HCLK_Div2); // APB1时钟 = 36MHz
//    
//    /*设置高速APB时钟（PCLK2）*/   
//    RCC_PCLK2Config(RCC_HCLK_Div1); // APB2时钟 = 72MHz
//    
//    /*FLASH时序配置（72MHz时需要2个延迟周期）*/   
//    FLASH_SetLatency(FLASH_Latency_2); 
//    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); // 启用预取指缓存
//    
//    // 使能PLL并等待稳定
//    RCC_PLLCmd(ENABLE);	
//    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); // 等待PLL锁定
//    
//    // 选择PLL作为系统时钟源
//    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); 
//    while(RCC_GetSYSCLKSource() != 0x08); // 等待系统时钟切换完成
//    
//    /*外设时钟使能（根据实际需求开启）*/   
//    // RCC_APB2PeriphClockCmd(...);  
//    // RCC_APB1PeriphClockCmd(...);  
//} 
