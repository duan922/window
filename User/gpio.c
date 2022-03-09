
#include "gpio.h"

/**
* @brief  �����õ���GPIO
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/
void GpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��PA�˿�ʱ��
 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);      /*ʹ��SWD ����JTAG*/
	 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;       //esp��λ                      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
    GPIO_ResetBits(GPIOA,GPIO_Pin_15);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
//	GPIO_Init(GPIOC, &GPIO_InitStructure);					     //�����趨������ʼ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		     //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //�����趨������ʼ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					     //�����趨������ʼ��
    GPIO_ResetBits(GPIOD,GPIO_Pin_7);
    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6;       //LED��                      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
    GPIO_SetBits(GPIOE,GPIO_Pin_6);
     
}
/*1�������� 2�������Ҹ� 3;ͣ*/
/*��������out3.4-PD12.13*/
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
/*���ȿ���out5.6-PD10.11*/
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
/*��Ҷ������out3.4-PD8.9*/
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
/*�ƵĿ���out1.2-PD14.15*/
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
/*�Ƶ���ɫ*/
void light(int num)
{
    switch(num)
    {
        case 1:                 //��
        {
            GPIO_SetBits(GPIOD,GPIO_Pin_4);
            GPIO_ResetBits(GPIOD,GPIO_Pin_5);
            GPIO_ResetBits(GPIOD,GPIO_Pin_6);
        }
            break;
        case 2:                 //��
        {
            GPIO_ResetBits(GPIOD,GPIO_Pin_4);
            GPIO_SetBits(GPIOD,GPIO_Pin_5);
            GPIO_ResetBits(GPIOD,GPIO_Pin_6);
        }
            break;
        case 3:                 //��
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


