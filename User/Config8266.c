/**
  ******************************************************************************
  * @author  yang feng wu 
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   ����8266
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


char SmartConfigFlage = 0;//�ǲ���������
u32  SmartConfigCnt = 0;//��������·������ʱʹ��
char SmartConfigSuccess = 0;//�Ƿ������ɹ�
u32  Config8266LedDelay=0;//����8266ָʾ����ҫ



char ThisSSID[32]="";//��¼·��������
char ThisPWD[64]="";//��¼����
char ThisMAC[18]="";//��¼�豸MAC
char ThisIP[21]="0.0.0.0";//��¼�豸����·�����ֵõ�IP


/**
* @brief  ����APUConfig ��WIFI����
* @ warn  None
* @param  None
* @param  None
* @param  None
* @param  None
* @retval 1:�ɹ�
* @example 
**/
char APUConfig(void)
{
//	SmartConfigPinOut = 1;
	SmartConfigSuccess = 0;
    light(3);//����ʱ�����
	Rst8266();
	if(ConfigModuleBlock("+++","+++",NULL))//�˳�͸��
	{
		if(ConfigModuleBlock("AT+RESTORE\r\n","ready",NULL))//�ָ���������
		{
			if(ConfigModuleBlock("AT+CWMODE_DEF=3\r\n","OK",NULL))//ģʽ3
			{
				if(ConfigModuleBlock("AT+CIPSTAMAC_CUR?\r\n","MAC_CUR",NULL))//��ѯMAC
				{						
					MainString = StrBetwString(Usart4BufferMemoryCopy,"MAC_CUR:\"","\"");//�õ�MAC
					if(strlen(MainString) ==17)
					{
						memset(ThisMAC,0,sizeof(ThisMAC));//�ڴ�����
						memcpy(ThisMAC,MainString,17);
					}
					else {goto end;}
					cStringRestore();					
					if(ConfigModuleBlock("AT+CWSAP_DEF=\"xxl\",\"123456789\",11,4,4\r\n","OK",NULL))//���÷���������"wifi_8266_bind\",\"11223344
					{
						if(ConfigModuleBlock("AT+CIPSTART=\"UDP\",\"192.168.4.2\",5555,5556,2\r\n","OK",NULL))//����UDP
						{
                        
                            SmartConfigCnt =0;//111111							
							while(1)
							{
								IWDG_Feed();//ι��                                
                              	Rst8266PinOut = 0;//����������λ
								BufferManageRead(&buff_manage_struct_usart4,Usart4BufferMemoryCopy,&buff_manage_struct_usart4.ReadLen);
								if(buff_manage_struct_usart4.ReadLen>0)//������
                                {
//									 printf("in\r\n");
									MainString = StrBetwString(Usart4BufferMemoryCopy,"\"ssid\":\"","\"");//��ȡssid�˺�
									if(MainString!=NULL) 
									{
										memset(ThisSSID,0,sizeof(ThisSSID));
										sprintf(ThisSSID,"%s",MainString);
										cStringRestore();										
										MainString = StrBetwString(Usart4BufferMemoryCopy,"\"pwd\":\"","\"");//��ȡpwd����
										if(MainString!=NULL) 
										{
											memset(ThisPWD,0,sizeof(ThisPWD));
											sprintf(ThisPWD,"%s",MainString);
											cStringRestore();//�ַ���ȫ
											break;
										}
										else {goto end;}
									}
//									else {goto end;}
								}			
																
							}
//							/*�����ǻ�ȡ�˷�����·�������ƺ�����*/
							
			
							/*�����豸MAC*/
							MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
							MainLen = sprintf(MainBuffer,"AT+CIPSEND=%d\r\n",MainLen);
							if(ConfigModuleBlock(MainBuffer,">",NULL))//׼����UDP�ͻ��˷�����Ϣ
							{
								memset(MainBuffer,0,sizeof(MainBuffer));
								MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
								UsartOutStrIT(UART4,MainBuffer,MainLen);
								
								SmartConfigCnt = 0;
								while(SmartConfigCnt<2000)
								{
									IWDG_Feed();//ι��
								}
							}else {goto end;}
							
							MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
							MainLen = sprintf(MainBuffer,"AT+CIPSEND=%d\r\n",MainLen);
							if(ConfigModuleBlock(MainBuffer,">",NULL))//׼����UDP�ͻ��˷�����Ϣ
							{
								memset(MainBuffer,0,sizeof(MainBuffer));
								MainLen = sprintf(MainBuffer,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
								UsartOutStrIT(UART4,MainBuffer,MainLen);
								
								SmartConfigCnt = 0;
								while(SmartConfigCnt<2000)
								{
									IWDG_Feed();//ι��
								}
							}else {goto end;}
							
							if(ConfigModuleBlock("AT+CWAUTOCONN=1\r\n","OK",NULL))//�Զ�����·����
							{
								sprintf(MainBuffer,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",ThisSSID,ThisPWD);
								if(ConfigModuleBlock(MainBuffer,"WIFI GOT IP","WIFI CONNECTED"))//�������ӵ�·����
								{
									SmartConfigSuccess=1;//�����ɹ�
								}
							}
						}
					}
				}
			}
		}
	}
	end:
	Rst8266();//��λ
	if(ConfigModuleBlock("AT+CWMODE_DEF=1\r\n","OK",NULL))//ģʽ1
	{SmartConfigFlage = 0;}
	
	return SmartConfigSuccess;
}

/**
* @brief  ��λ8266
* @ warn  ��Ƭ�� Rst8266Pin ������Ҫ��8266 RST����������
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
* @brief  ����·����
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
	ConfigModuleBlock("+++","+++",NULL);//�˳�͸��
	ConfigModuleBlock("AT+RESTORE\r\n","ready",NULL);//�ظ���������
	ConfigModuleBlock("AT+CWMODE_DEF=1\r\n","OK",NULL);//ģʽ1
	ConfigModuleBlock("AT+CWAUTOCONN=1\r\n","OK",NULL);//�Զ�����·����
    ConfigModuleBlock("AT+CWJAP_DEF=\"1002\",\"1234567890\"\r\n","OK","WIFI CONNECTED");//�������ӵ�·����
	Rst8266();//��λ
}


/**
* @brief  ����TCP������
* @ warn  None
* @param  None
* @param  None
* @param  None
* @param  None
* @retval 1:��������TCP������
* @example 
**/
char AutoConnectTCP(void)
{
	char flage=0;
	char data[100];
	Rst8266();
	ConfigModuleBlock("+++","+++",NULL);//�˳�͸��
	sprintf(data,"AT+SAVETRANSLINK=1,\"%s\",%s,\"TCP\"\r\n","116.62.44.118","8083");
	flage = ConfigModuleBlock(data,"OK","ALREADY CONNECTED");//����TCP������
	Rst8266();
	return flage;
}


