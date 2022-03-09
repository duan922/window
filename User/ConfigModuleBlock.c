/**
  ******************************************************************************
  * @file    usart.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   
  ******************************************************************************
	一,使用说明
		1,把以下程序放在1ms定时器中断中
			ConfigModuleBlockDelay++;
		2,使用
		//发送AT\r\n,希望串口返回的数据是 OK,
		ConfigModuleBlock("AT\r\n","OK",NULL) //判断返回了OK,或者该指令发送了3次,执行下一条
		
		//发送AT+RST\r\n,希望串口返回的数据是 OK,
		ConfigModuleBlock("AT+RST\r\n","ready","OK") //判断返回了OK,或者返回了"ready",或者该指令发送了3次,执行下一条
		
		//接收返回的配置状态
		flage = ConfigModuleBlock("AT\r\n","OK",NULL);//flage:1  配置OK  
  ******************************************************************************
  */

//#define CONFIGMODULEBLOCK_C_

#include <string.h>
#include <stdio.h>

#include "iwdg.h"
#include "BufferManage.h"
#include "usart.h"


int  ConfigModuleBlockCnt = 0;
int	 ConfigModuleBlockFlage = 1;
int  ConfigModuleBlockDelay=0;

/**
* @brief  发送指令配置模块,阻塞版
* @param  dat:      发送的数据
* @param  returnc:  预期返回的数据1
* @param  returncc: 预期返回的数据2
* @retval 1:配置当前指令OK
* @example 
**/
char ConfigModuleBlock(char *dat,char *returnc,char *returncc)
{
	ConfigModuleBlockCnt = 0;
	ConfigModuleBlockFlage = 1;
	while(1)
	{
		IWDG_Feed();//喂狗
		if(ConfigModuleBlockFlage == 1)//发送指令
		{
			ConfigModuleBlockDelay = 0;
			memset(Usart4BufferMemoryCopy,0,buff_manage_struct_usart4.SendLen);
			ConfigModuleBlockFlage = 0;			
			UsartOutStrIT(UART4,(unsigned char*)dat,strlen(dat));
        }
		
		BufferManageRead(&buff_manage_struct_usart4,Usart4BufferMemoryCopy,&buff_manage_struct_usart4.ReadLen);
		if(buff_manage_struct_usart4.ReadLen>0){//有数据
			if(returnc!=NULL && strstr((char*)Usart4BufferMemoryCopy,returnc))//比较数据
			{
				return 1;
			}
			if(returncc!=NULL && strstr((char*)Usart4BufferMemoryCopy,returncc))//比较数据
			{
				return 1;
			}
		}
		
		if(ConfigModuleBlockDelay>=3000)//超过3S
		{
			ConfigModuleBlockDelay = 0;
			ConfigModuleBlockFlage = 1;//允许发送数据
			ConfigModuleBlockCnt++;
		}
		if(ConfigModuleBlockCnt>=3)//超过三次继续发送下一条
		{
			ConfigModuleBlockCnt = 0;
			return 0;
		}
  }
}

