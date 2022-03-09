
#include "gpio.h"

/**
* @brief  设置用到的GPIO
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/
void GpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);	 //使能PA端口时钟
 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);      /*使能SWD 禁用JTAG*/
	 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;       //esp复位                      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
    GPIO_ResetBits(GPIOA,GPIO_Pin_15);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
//	GPIO_Init(GPIOC, &GPIO_InitStructure);					     //根据设定参数初始化
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		     //下拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					     //根据设定参数初始化
    GPIO_ResetBits(GPIOD,GPIO_Pin_7);
    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6;       //LED灯                      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
    GPIO_SetBits(GPIOE,GPIO_Pin_6);
     
}
/*1：左负右正 2：左正右负 3;停*/
/*窗户正反out3.4-PD12.13*/
void windows(int nums)
{
    switch(nums)
    {
        case 1:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_12);
        GPIO_SetBits(GPIOD,GPIO_Pin_13);
        }break;
        case 2:
        {
        GPIO_SetBits(GPIOD,GPIO_Pin_12);
        GPIO_ResetBits(GPIOD,GPIO_Pin_13);
        }break;
        case 3:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_12);
        GPIO_ResetBits(GPIOD,GPIO_Pin_13);            
        }break;
    }
}
/*风扇开关out5.6-PD10.11*/
void fan_control(int nums)
{
    switch(nums)
    {
        case 1:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_10);
        GPIO_SetBits(GPIOD,GPIO_Pin_11);
        }break;
        case 2:
        {
        GPIO_SetBits(GPIOD,GPIO_Pin_10);
        GPIO_ResetBits(GPIOD,GPIO_Pin_11);
        }break;
        case 3:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_10);
        GPIO_ResetBits(GPIOD,GPIO_Pin_11);            
        }break;
    }
}
/*百叶窗开关out3.4-PD8.9*/
void blind_control(int nums)
{
    switch(nums)
    {
        case 1:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_8);
        GPIO_SetBits(GPIOD,GPIO_Pin_9);
        }break;
        case 2:
        {
        GPIO_SetBits(GPIOD,GPIO_Pin_8);
        GPIO_ResetBits(GPIOD,GPIO_Pin_9);
        }break;
        case 3:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_8);
        GPIO_ResetBits(GPIOD,GPIO_Pin_9);            
        }break;
    }
}
/*灯的开关out1.2-PD14.15*/
void light_control(int nums)
{
    switch(nums)
    {
        case 1:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_14);
        GPIO_SetBits(GPIOD,GPIO_Pin_15);
        }break;
        case 2:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_14);
        GPIO_SetBits(GPIOD,GPIO_Pin_15);
        }break;
        case 3:
        {
        GPIO_ResetBits(GPIOD,GPIO_Pin_14);
        GPIO_ResetBits(GPIOD,GPIO_Pin_15); 
        }break;
      
    }
}
/*灯的颜色*/
void light(int num)
{
    switch(num)
    {
        case 1:                 //蓝
        {
            GPIO_SetBits(GPIOD,GPIO_Pin_4);
            GPIO_ResetBits(GPIOD,GPIO_Pin_5);
            GPIO_ResetBits(GPIOD,GPIO_Pin_6);
        }
            break;
        case 2:                 //绿
        {
            GPIO_ResetBits(GPIOD,GPIO_Pin_4);
            GPIO_SetBits(GPIOD,GPIO_Pin_5);
            GPIO_ResetBits(GPIOD,GPIO_Pin_6);
        }
            break;
        case 3:                 //红
        {
            GPIO_ResetBits(GPIOD,GPIO_Pin_4);
            GPIO_ResetBits(GPIOD,GPIO_Pin_5);
            GPIO_SetBits(GPIOD,GPIO_Pin_6);
        }
            break;
        default:
            {
            GPIO_ResetBits(GPIOD,GPIO_Pin_4);
            GPIO_ResetBits(GPIOD,GPIO_Pin_5);
            GPIO_ResetBits(GPIOD,GPIO_Pin_6);
        }
            break;
    }
}


