
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "System.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "ConfigModuleNoBlock.h"
#include "Config8266.h"
#include "timer.h"
#include "iwdg.h"
#include "key.h"
#include "DHT11.h"
#include "oled.h"
#include "mqtt.h"
#include "usart3.h"
#include "pwm.h"
#include "adc.h"


//缓存数据使用
unsigned char MainBuffer[MainBufferLen];//缓存数据,全局通用
unsigned char *MainString;    //全局通用变量
u32  MainLen=0;      //全局通用变量
u32  Gas=0;
u32  bee_time;
u32  windnum=0;//开关定时
u32  windnums=0;//微调定时
u32  blindnum=0;//开关定时
u32  lengh_timme=0;

char wind_flag=0;//窗户标志位
char wind_status=0,blind_status=0,fan_status=0,uv_status=0;//状态显示
u16 CO2,CH2O,TVOC,PM25,PM10,numb;
float TEM,HUM,voltage,CO,SMOKE;

char RelayState = 0;//记录继电器状态


//连接MQTT
unsigned char IP[55]="47.97.183.24";//IP地址/域名47.97.183.24  116.62.44.118
unsigned char Port[7] = "1884";//端口号
unsigned char MQTTid[50] = "";//ClientID
unsigned char MQTTUserName[20] = "admin";//用户名
unsigned char MQTTPassWord[50] = "public";//密码
unsigned char MQTTkeepAlive = 30;//心跳包时间

unsigned char MQTTPublishTopic[30]="";//存储MQTT发布的主题
unsigned char MQTTPublishTopic1[30]="device";
unsigned char MQTTWillMessage[50] = "{\"Disconnect...\"}";//遗嘱消息
unsigned char MQTTWillQos = 0;//消息等级
unsigned char MQTTWillRetained = 1;//是否需要服务器记录

unsigned char MQTTSubscribeTopic[30]="";//存储MQTT订阅的主题
unsigned char MQTTSubscribeTopic1[30]="user";//存储MQTT订阅的主题
unsigned char MQTTSubscribeTopic2[30]="";//存储MQTT订阅的主题




void APUConfigFunction(void);//APUConfig配网
void DHT11Function(void);//DHT11,温湿度采集
void RelayFunction(void);//继电器状态检测

void GasFunction(void);//气体检测
void safety(void);
void MqttConnect(void);//连接上MQTT回调
void MqttDisConnect(void);//MQTT断开连接回调
void subscribedCb(int pdata);//订阅成功回调
void failsubscribedCb(int pdata);//订阅失败回调
void PublishedCb(void);//成功发布消息回调
void MqttReceive(const char* topic, uint32_t topic_len,const char *data, uint32_t lengh);//接收到数据回调


int main(void)
{
  NVIC_Configuration();
	uart_init(115200,115200);	 //串口初始化为115200
    usart3_init(9600);
	GpioInit();
    TIM3_PWM_Init();
	DelayInit();
	Timer2_Config();
    Adc_Init();
	IWDG_Init(IWDG_Prescaler_256,255*5);
	
	/*MQTT*/
	mqtt_init(&mymqtt);
	mqtt_connect_reg(&mymqtt,MqttConnect);//注册连接回调函数
	mqtt_disconnect_reg(&mymqtt,MqttDisConnect);//注册断开连接回调函数
	mqtt_received_reg(&mymqtt,MqttReceive);//注册接收数据回调函数
    light(blue);//蓝
    printf("start");
//   Init8266();
	while(1)
	{
		IWDG_Feed();//喂狗          
		if(!SmartConfigFlage)
		{
			ConfigModuleNoBlock();//配置模块连接MQTT服务器      
		}
		APUConfigFunction(); //APU配网
		if(ConfigModuleNoBlockFlage)//(模块连接了MQTT)
		{
			mqtt_send_function(&mymqtt);//提取发送缓存的MQTT协议
			mqtt_keep_alive(&mymqtt);//处理发送心跳包			
		}
		                                               
		//读取串口接收的数据
		BufferManageRead(&buff_manage_struct_usart4,Usart4BufferMemoryCopy,&buff_manage_struct_usart4.ReadLen);
		if(buff_manage_struct_usart4.ReadLen>0)//模块返回了数据
		{
			if(ConfigConnectDispose != NULL)
			{
				ConfigConnectDispose((char*)Usart4BufferMemoryCopy);//处理模块返回的数据
				memset(Usart4BufferMemoryCopy,0,buff_manage_struct_usart4.ReadLen);
			}
			
			if(ConfigModuleNoBlockFlage)//(模块连接了MQTT)
			{
				if(lengh_timme>15000)
				{			
					printf("............\r\n");
						ConfigModuleNoBlockCaseValue=0;
						ConfigModuleNoBlockFlage = 0;	
				
				}
				mqtt_read_function(&mymqtt,Usart4BufferMemoryCopy,buff_manage_struct_usart4.ReadLen);//函数处理MQTT数据
               
			}        
		}	
		
        GasFunction();
//    
	}
}

/**
* @brief   APUConfig配网
* @param   None
* @example
**/
void APUConfigFunction(void)
{
	SmartConfigKey();
	if(KeySmartConfig[3] == 1)
	{
        if(KeySmartConfig[5]==3000)
        {
			KeySmartConfig[3] = 0;//清零以后,只有按键松开再按下的时候才会进入
			if(!SmartConfigFlage)//如果SmartConfigFlage=0，没在配网
			{
				SmartConfigFlage = 1;
				APUConfig();//配网
				ConfigModuleNoBlockFlage=0;
				ConfigModuleNoBlockCaseValue=0;//启用配置WIFI函数
			}
        }
	}
}

/**
* @brief   安全检测
**/
void safety(void)
{
    int vnum,gas_num1,gas_num2;
    CO2=0;CH2O=0;TVOC=0;PM25=0;
    vnum=Get_Adc(15);//pc5
    gas_num1=Get_Adc(8);//pb0
    gas_num2=Get_Adc(9);//pb1
    voltage=vnum*3/4096.00;//推窗电压
    CO=gas_num1*3/4096.00;//CO电压
    SMOKE=gas_num2*3/4096.00;//烟雾电压
    CO2=rx3buffer[2]*256+rx3buffer[3];//计算CO2值
    CH2O=rx3buffer[4]*256+rx3buffer[5];//计算CH2O值
    TVOC=rx3buffer[6]*256+rx3buffer[7];//计算TVOC值
    PM25=rx3buffer[8]*256+rx3buffer[9];//计算PM25值
    PM10=rx3buffer[10]*256+rx3buffer[11];//计算PM10值      
    TEM=(rx3buffer[12]*10+rx3buffer[13])/10.0;//计算TEM值
    HUM=(rx3buffer[14]*10+rx3buffer[15])/10.0;//计算HUM值
//    if(PM25>150)
//    {
//        if(bee_time<=3000)BEE=1;
//        else BEE=0;
//        
//    }else BEE=0;
    if(voltage>1.90)
    {
        windows(stop);
    }

}

void GasFunction(void)//气体检测
{
    if(Gas>=1500)
	{
        Gas=0;
        safety(); 
        if(ConfigModuleNoBlockFlage)//配置模块成功(模块连接了MQTT)
		{
            rx3_cnt=0;
            MainLen = sprintf((char*)MainBuffer,"{\"MAC\":%s,\"CO2\":%d,\"CH2O\":%d,\"TV\":%d,\"P25\":%d,\"P10\":%d,\"TEM\":%2.1f,\"HUM\":%2.1f,\"CO\":%1.2f,\"S\":%1.2f,\"VOL\":%1.3f,\"T\":%d,\"B\":%d,\"F\":%d,\"UV\":%d}",&MQTTid[0],CO2,CH2O,TVOC,PM25,PM10,TEM,HUM,CO,SMOKE,voltage,wind_status,blind_status,fan_status,uv_status);                               
			mqtt_publish(&mymqtt,MQTTPublishTopic,MainBuffer,MainLen, 0, 0,PublishedCb);//发布消息
		}
         
    }
}

/**
* @brief   连接上MQTT回调函数
* @param   None
* @retval  None
* @warning None
* @example
**/
void MqttConnect()
{
	memset(MQTTSubscribeTopic,NULL,sizeof(MQTTSubscribeTopic));
	sprintf((char*)MQTTSubscribeTopic,"%s%s","user/",&MQTTid[0]);//组合字符串
	mqtt_subscribe(&mymqtt, MQTTSubscribeTopic, 0,subscribedCb,failsubscribedCb);//订阅主题
	
	memset(MQTTSubscribeTopic,NULL,sizeof(MQTTSubscribeTopic));
	sprintf((char*)MQTTSubscribeTopic2,"%s%s","device/",&MQTTid[0]);//组合字符串
	mqtt_subscribe(&mymqtt, MQTTSubscribeTopic2, 0,subscribedCb,failsubscribedCb);//订阅主题
}

/**
* @brief   MQTT断开连接回调
* @param   None
* @retval  None
* @warning None
* @example
**/ 
void MqttDisConnect()
{
	mqtt_init(&mymqtt);	
	//重新配置模块连接MQTT
	ConfigModuleNoBlockCaseValue=0;
	ConfigModuleNoBlockFlage = 0;	
	light(blue);
	printf("\r\n连接断开**********************************************************\r\n");
}


/**
* @brief   MQTT接收数据回调
* @param   topic:主题
* @param   topic_len:主题长度
* @param   data:接收的数据
* @param   lengh:接收的数据长度
* @retval  None
* @warning None
* @example
**/
void MqttReceive(const char* topic, uint32_t topic_len,const char *data, uint32_t lengh)
{	
	lengh_timme=0;
	if(strstr((char*)data, "\"data\":\"switch\""))//询问开关user/8c:aa:b5:71:61:ff  device/8c:aa:b5:71:61:ff device/e0:98:06:0c:02:18{"data":"switch","bit":"1","status":"1"}
	{
        /*控制推窗开关*/
		if(strstr((char*)data, "\"bit\":\"1\""))//第一路开关
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//关
			{
                if(wind_flag!=1)
                {
                    windnum=0;
                    windows(fanz);
                    wind_status=0;
                    wind_flag=1;
                }
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//开
			{
                if(wind_flag!=1)
                {
                    windnum=0;
                    windows(zhengz);
                    wind_status=1;
                    wind_flag=1;
                }
			}
            else if(strstr((char*)data, "\"status\":\"-1\""))//停
			{              
                    windnums=0;
                    windows(fanz);
                    wind_status=0;
                    wind_flag=2;
                   
			}
			
		}
        /*控制百叶窗开关*/
        if(strstr((char*)data, "\"bit\":\"2\""))//第二路开关//询问开关{"data":"switch","bit":"2","status":"1"}
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//关
			{
                blindnum=0;
				blind_control(zhengz);
                blind_status=0;
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//开
			{
                blindnum=0;
				blind_control(fanz);
				blind_status=1;
			}
            else if(strstr((char*)data, "\"status\":\"-1\""))
            {
                blind_control(stop);
//				blind_status=0;
            }
		
		}
        /*控制风扇开光*/
        if(strstr((char*)data, "\"bit\":\"3\""))//第三路开关询问开关{"data":"switch","bit":"3","status":"1"}
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//关
			{
				fan_control(stop);
				fan_status=0;
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//开
			{
				fan_control(zhengz);
				fan_status=1;
			}
		
		}
        /*控制紫外灯*/
        if(strstr((char*)data, "\"bit\":\"4\""))//第四路开关询问开关{"data":"switch","bit":"4","status":"1"}
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//关
			{
				light_control(stop);
				uv_status=0;
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//开
			{
				light_control(zhengz);
				uv_status=1;
			}		
		}
	}
	
}


/**
* @brief   订阅主题成功
* @param   None
* @retval  None
* @warning None
* @example
**/
void subscribedCb(int pdata)
{
	printf("\r\n成功订阅主题\r\n");
}

/**
* @brief   订阅主题失败
* @param   None
* @retval  None
* @warning None
* @example
**/
void failsubscribedCb(int pdata)
{
	printf("\r\n订阅主题失败\r\n");
}


/**
* @brief   发布成功
* @param   None
* @retval  None
* @warning None
* @example
**/
void PublishedCb()
{
	printf("\r\n发布成功\r\n");
}



