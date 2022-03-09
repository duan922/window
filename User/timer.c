
/**
  ******************************************************************************
  * @file    key.c
  * @author  yang feng wu 
  * @version V1.0.0
  * @date    2019/10/13
  * @brief   字符串处理函数
  ******************************************************************************
	1.找个位置写上以下程序
	//定时器2
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
  ******************************************************************************
  */


#define _TIME_C_


#include "timer.h"
#include "ConfigModuleNoBlock.h"
#include "ConfigModuleBlock.h"
#include "BufferManage.h"
#include "mqtt.h"
#include "DHT11.h"
#include "key.h"
#include "Config8266.h"
#include "main.h"
#include "gpio.h"
#include "stdio.h"

/**
* @brief  定时器2配置
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/

void Timer2_Config(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Resets the TIM2 */
  TIM_DeInit(TIM2);
	//设置了时钟分割。
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	// 选择了计数器模式。
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//初值
	TIM_TimeBaseStructure.TIM_Period = 10;//定时时间1ms进一次
	//设置了用来作为 TIMx 时钟频率除数的预分频值。72M / 7199+1 = 0.01M
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;
	
	//TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	/* Enables the TIM2 counter */
  TIM_Cmd(TIM2, ENABLE);
	
	/* Enables the TIM2 Capture Compare channel 1 Interrupt source */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE );
}

/**
* @brief  定时器2中断函数
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/
int a=0;
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
//		RendTHCnt++;
//        
//		a++;
//		printf("%d\r\n",a);
//		if(a==5)
//		{
//			light(red);//hong
//			a=0;
//		}
		Gas++;
        bee_time++;
        windnums++;
        windnum++;
        blindnum++;
		lengh_timme++;
		ConfigModuleBlockDelay++;
		
		ConfigModuleNoBlockCnt++;
		
		mqtt_time_data(&mymqtt);/*定时轮训函数*/
        if(bee_time==10000)
        {bee_time=0;}
		
		/*定时开窗*/
        if(windnum==400)
        {
            windnum=0;
            wind_flag=0;
            windows(stop);
        }
        if((windnums>=50)&&(wind_flag==2))
        {
            windnums=0;
            windows(stop);
        }
       /*定时开窗*/
        if(blindnum==60)
        {
            blindnum=0;
            blind_control(stop);
        }
        
		/*配置模块SmartConfig,连接TCP*/
		SmartConfigCnt++;
				if(SmartConfigCnt>70000)
				{
					SmartConfigCnt=0;
				}
		if(SmartConfigFlage)//配网状态,指示灯闪耀
		{
			Config8266LedDelay++;
			if(Config8266LedDelay>100)
			{
				Config8266LedDelay=0;
				SmartConfigPinOut = ~SmartConfigPinOut;
                light(red);
			}
		}
		else
		{
			if(ConfigModuleNoBlockFlage == 1)//模块配置连接了TCP服务器
			{
				Config8266LedDelay++;
				if(Config8266LedDelay>1000)//1S闪耀
				{
					Config8266LedDelay=0;
					SmartConfigPinOut = ~SmartConfigPinOut;
                    light(green);
				}
			}
			else
			{
				Config8266LedDelay=0;
			}
		}				
		
		/*按键处理*/
		if(KeySmartConfig[1])
			KeySmartConfig[5]++;
		else
			KeySmartConfig[5]=0;

		if(KeySmartConfig[2])
			KeySmartConfig[6]++;
		else
			KeySmartConfig[6]=0;
	}
}





