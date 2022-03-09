/**
  ******************************************************************************
  * @author  yang feng wu 
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   配置8266
  ******************************************************************************
  ******************************************************************************
  */

#define CONFIG8266_C_
#include <stdio.h>
#include <string.h>

#include "iwdg.h"
#include "Config8266.h"
#include "ConfigModuleBlock.h"

#include "BufferManage.h"
#include "usart.h"
#include "delay.h"
#include "main.h"
#include "cString.h"
#include "gpio.h"
#include "main.h"


char SmartConfigFlage = 0;//是不是在配网
u32  SmartConfigCnt = 0;//配网连接路由器延时使用
char SmartConfigSuccess = 0;//是否配网成功
u32  Config8266LedDelay=0;//配置8266指示灯闪耀



char ThisSSID[32]="";//记录路由器名称
char ThisPWD[64]="";//记录密码
char ThisMAC[18]="";//记录设备MAC
char ThisIP[21]="0.0.0.0";//记录设备连接路由器分得的IP


/**
* @brief  启用APUConfig 给WIFI配网
* @ warn  None
* @param  None
* @param  None
* @param  None
* @param  None
* @retval 1:成功
* @example 
**/
char APUConfig(void)
{
//	SmartConfigPinOut = 1;
	SmartConfigSuccess = 0;
    light(3);//配置时亮红灯
	Rst8266();
	if(ConfigModuleBlock("+++","+++",NULL))//退出透传
	{
		if(ConfigModuleBlock("AT+RESTORE\r\n","ready",NULL))//恢复出厂设置
		{
			if(ConfigModuleBlock("AT+CWMODE_DEF=3\r\n","OK",NULL))//模式3
			{
				if(ConfigModuleBlock("AT+CIPSTAMAC_CUR?\r\n","MAC_CUR",NULL))//查询MAC
				{						
					MainString = StrBetwString(Usart4BufferMemoryCopy,"MAC_CUR:\"","\"");//得到MAC
					if(strlen(MainString) ==17)
					{
						memset(ThisMAC,0,sizeof(ThisMAC));//内存清理
						memcpy(ThisMAC,MainString,17);
					}
					else {goto end;}
					cStringRestore();					
					if(ConfigModuleBlock("AT+CWSAP_DEF=\"xxl\",\"123456789\",11,4,4\r\n","OK",NULL))//配置发出的无线"wifi_8266_bind\",\"11223344
					{
						if(ConfigModuleBlock("AT+CIPSTART=\"UDP\",\"192.168.4.2\",5555,5556,2\r\n","OK",NULL))//配置UDP
						{
                        
                            SmartConfigCnt =0;//111111							
							while(1)
							{
								IWDG_Feed();//喂狗                                
                              	Rst8266PinOut = 0;//不能让它复位
								BufferManageRead(&buff_manage_struct_usart4,Usart4BufferMemoryCopy,&buff_manage_struct_usart4.ReadLen);
								if(buff_manage_struct_usart4.ReadLen>0)//有数据
                                {
//									 printf("in\r\n");
									MainString = StrBetwString(Usart4BufferMemoryCopy,"\"ssid\":\"","\"");//获取ssid账号
									if(MainString!=NULL) 
									{
										memset(ThisSSID,0,sizeof(ThisSSID));
										sprintf(ThisSSID,"%s",MainString);
										cStringRestore();										
										MainString = StrBetwString(Usart4BufferMemoryCopy,"\"pwd\":\"","\"");//获取pwd密码
										if(MainString!=NULL) 
										{
											memset(ThisPWD,0,sizeof(ThisPWD));
											sprintf(ThisPWD,"%s",MainString);
											cStringRestore();//字符补全
											break;
										}
										else {goto end;}
									}
//									else {goto end;}
								}			
																
							}
//							/*以上是获取了发来的路由器名称和密码*/
							
			
							/*返回设备MAC*/
							MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
							MainLen = sprintf(MainBuffer,"AT+CIPSEND=%d\r\n",MainLen);
							if(ConfigModuleBlock(MainBuffer,">",NULL))//准备向UDP客户端发送消息
							{
								memset(MainBuffer,0,sizeof(MainBuffer));
								MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
								UsartOutStrIT(UART4,MainBuffer,MainLen);
								
								SmartConfigCnt = 0;
								while(SmartConfigCnt<2000)
								{
									IWDG_Feed();//喂狗
								}
							}else {goto end;}
							
							MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
							MainLen = sprintf(MainBuffer,"AT+CIPSEND=%d\r\n",MainLen);
							if(ConfigModuleBlock(MainBuffer,">",NULL))//准备向UDP客户端发送消息
							{
								memset(MainBuffer,0,sizeof(MainBuffer));
								MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
								UsartOutStrIT(UART4,MainBuffer,MainLen);
								
								SmartConfigCnt = 0;
								while(SmartConfigCnt<2000)
								{
									IWDG_Feed();//喂狗
								}
							}else {goto end;}
							
							if(ConfigModuleBlock("AT+CWAUTOCONN=1\r\n","OK",NULL))//自动连接路由器
							{
								sprintf(MainBuffer,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",ThisSSID,ThisPWD);
								if(ConfigModuleBlock(MainBuffer,"WIFI GOT IP","WIFI CONNECTED"))//设置连接的路由器
								{
									SmartConfigSuccess=1;//配网成功
								}
							}
						}
					}
				}
			}
		}
	}
	end:
	Rst8266();//复位
	if(ConfigModuleBlock("AT+CWMODE_DEF=1\r\n","OK",NULL))//模式1
	{SmartConfigFlage = 0;}
	
	return SmartConfigSuccess;
}

/**
* @brief  复位8266
* @ warn  单片机 Rst8266Pin 引脚需要和8266 RST引脚相连接
* @param  None
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/
void Rst8266(void)
{
	Rst8266PinOut = 1;
	delay_ms(100);
	Rst8266PinOut = 0;
	delay_ms(100);
}


/**
* @brief  连接路由器
* @ warn  None
* @param  None
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/
void Init8266(void)
{
	Rst8266();
	ConfigModuleBlock("+++","+++",NULL);//退出透传
	ConfigModuleBlock("AT+RESTORE\r\n","ready",NULL);//回复出厂设置
	ConfigModuleBlock("AT+CWMODE_DEF=1\r\n","OK",NULL);//模式1
	ConfigModuleBlock("AT+CWAUTOCONN=1\r\n","OK",NULL);//自动连接路由器
    ConfigModuleBlock("AT+CWJAP_DEF=\"1002\",\"1234567890\"\r\n","OK","WIFI CONNECTED");//设置连接的路由器
	Rst8266();//复位
}


/**
* @brief  链接TCP服务器
* @ warn  None
* @param  None
* @param  None
* @param  None
* @param  None
* @retval 1:可以连接TCP服务器
* @example 
**/
char AutoConnectTCP(void)
{
	char flage=0;
	char data[100];
	Rst8266();
	ConfigModuleBlock("+++","+++",NULL);//退出透传
	sprintf(data,"AT+SAVETRANSLINK=1,\"%s\",%s,\"TCP\"\r\n","116.62.44.118","8083");
	flage = ConfigModuleBlock(data,"OK","ALREADY CONNECTED");//连接TCP服务器
	Rst8266();
	return flage;
}


