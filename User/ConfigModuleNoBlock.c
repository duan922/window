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
			ConfigModuleNoBlockCnt++;
			
		2,把以下程序放到主函数
		while(1)
		{
			ConfigModuleNoBlock();//配置模块
			
			if(串口接收到一条完整的数据)
			{
			//把数据放到: ConfigConnectDispose函数处理  data:数据地址
			  ConfigConnectDispose(data);
			}
		}
			
  ******************************************************************************
  */


#define CONFIGMODULENOBLOCK_C_
#include <stdio.h>

#include "ConfigModuleNoBlock.h"
#include "main.h"
#include "cString.h"
#include "usart.h"
#include "Config8266.h"
#include "mqtt.h"


u32 ConfigModuleNoBlockCnt =0;        //配置函数延时变量,定时器内部累加
int ConfigModuleNoBlockCaseValue = 0; //控制执行哪一条Case 语句
char ConfigModuleNoBlockFlage  = 0;   //1-配置完连接 0-未配置完连接

u32 CompareValue=5000;                //每隔 Ms 发送一次数据
u32 SendNextDelay =0;                 //接收SendConfigFunction函数最后一个参数,最终传递给 ConfigModuleNoBlockCnt 控制写一条数据发送的时间
int Cnt = 0;                          //记录运行状态发送的次数
char DataReturnFlage  = 0;            //是否返回了预期的数据

char HopeReturnData1[20]="";//存储希望返回的数据
char HopeReturnData2[20]="";//存储希望返回的数据

void (*ConfigConnectDispose)(char *data);//定义一个函数指针变量,用来处理模块返回的数据

/*********************************************************************************************************/



/**
* @brief  //获取唯一ID,WIFI获取MAC,GPRS获取IMEI
* @param  data  
* @param  
* @retval 
* @example 
**/
void FunctionParseGetID(char *data)
{
	MainString = StrBetwString(data,"MAC_CUR:\"","\"");
	
	if(strlen(MainString) ==17)
	{
		memset(MQTTid,0,sizeof(MQTTid));
		memcpy(MQTTid,MainString,17);
		//如果想使用自定义ID,请屏蔽以上两行
		
	  DataReturnFlage=1;
		ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt:SendNextDelay);
	}
	cStringRestore();
}


/**
* @brief  //组合MQTT协议,并发送AT+CIPSEND=XXX
* @param  
* @param 
* @retval 
* @example 
**/
void FunctionParseConnectMqtt(void)
{		
	unsigned char *str;
	int len;
	memset(MQTTPublishTopic,NULL,sizeof(MQTTPublishTopic));
	sprintf(MQTTPublishTopic,"%s%s","device/",&MQTTid[0]);//组合发布的主题
	
	
	mymqtt.mqtt_connect_info.client_id = MQTTid;//client_id
	mymqtt.mqtt_connect_info.keepalive = MQTTkeepAlive;//心跳包时间
	mymqtt.mqtt_connect_info.username = MQTTUserName;//用户名
	mymqtt.mqtt_connect_info.password = MQTTPassWord;///密码
	
	
	mymqtt.mqtt_connect_info.will_topic = MQTTPublishTopic;//遗嘱发布的主题
	mymqtt.mqtt_connect_info.will_message = MQTTWillMessage;//遗嘱的消息
	mymqtt.mqtt_connect_info.will_qos = MQTTWillQos;//遗嘱的消息等级
	mymqtt.mqtt_connect_info.will_retain = MQTTWillRetained;//是否需要服务器保留消息
	mymqtt.mqtt_connect_info.clean_session = 1;//清除连接信息
	
	len = mqtt_connect(&mymqtt,&str);//打包连接信息
	
	if(len>0){
		UsartOutStrIT(UART4,str,len);//发送MQTT协议数据
	}
}


/**
* @brief  //判断是否连接上MQTT
* @param  data:MQTT服务器返回的数据
* @param 
* @retval 
* @example 
**/
void FunctionParseConnectMqttAck(char *data)
{
	if(mqtt_connect_ack(data)==0)
	{
		DataReturnFlage=1;
		ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt:SendNextDelay);
		
		if(mymqtt.connectCb){//调用连接回调函数
			mymqtt.connectCb();
		}
	}
}


/**
* @brief  发送指令配置模块,非阻塞版
* @waring 
* @param  None 
* @param  None
* @param  None
* @retval None
* @example 
**/
void ConfigModuleNoBlock(void)
{
	if(ConfigModuleNoBlockCnt>CompareValue && ConfigModuleNoBlockFlage==0)//没配置完 和 配置函数延时变量>CompareValue
	{
		ConfigModuleNoBlockCnt=0;
		if(DataReturnFlage == 1)//上一条指令是OK的
		{
			Cnt=0;
			DataReturnFlage = 0;
			ConfigModuleNoBlockCaseValue ++;//执行下一条
		}
		else
		{
			Cnt ++;
			if(Cnt>=3)//超过3次重新执行
			{
				Cnt=0;
				ConfigModuleNoBlockCaseValue = 0;
			}
		}
		switch(ConfigModuleNoBlockCaseValue)
		{
			case 0://关闭WIFI模块  默认返回了数据,可以发送下一条  延时500ms发送下一条
				Rst8266PinOut=1;DataReturnFlage=1;ConfigModuleNoBlockCnt = CompareValue-500; 
				break;//
			case 1://启动WIFI模块  默认返回了数据,可以发送下一条
				Rst8266PinOut=0;DataReturnFlage=1;break;
			case 2://退出透传 																							返回成功,发送下一条数据的延时时间:100 (Ms) 			
				SendConfigFunction("+++",NULL,"+++",NULL,FunctionParseCompare,CompareValue-100);
				break;
			case 3://关闭回显                                      					返回成功,立即发送下一条数据
				SendConfigFunction("ATE0\r\n",NULL,"OK",NULL,FunctionParseCompare,CompareValue);
				break;
			case 4://获取唯一ID,WIFI获取MAC,GPRS获取IMEI
				SendConfigFunction("AT+CIPSTAMAC_CUR?\r\n",NULL,NULL,NULL,FunctionParseGetID,CompareValue);
				break;
			case 5://单链接
				SendConfigFunction("AT+CIPMUX=0\r\n",NULL,"OK",NULL,FunctionParseCompare,CompareValue);
				break;
			case 6://透传模式
				SendConfigFunction("AT+CIPMODE=1\r\n",NULL,"OK",NULL,FunctionParseCompare,CompareValue);
				break;
			case 7://连接TCP
				MainLen = sprintf(MainBuffer,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",IP,Port);
				MainBuffer[MainLen]=0;
				SendConfigFunction(MainBuffer,NULL,"CONNECT","ALREADY CONNECTED",FunctionParseCompare,CompareValue);
				break;
			case 8://启动透传
				SendConfigFunction("AT+CIPSEND\r\n",NULL,"OK",NULL,FunctionParseCompare,CompareValue);
				lengh_timme=0;
				break;
			case 9://连接MQTT
				SendConfigFunction(NULL,FunctionParseConnectMqtt,NULL,NULL,FunctionParseConnectMqttAck,CompareValue);
				break;
			default: 
				SendConfigFunction(NULL,NULL,NULL,NULL,NULL,CompareValue);//这句必须加,清除所有的执行函数
				DataReturnFlage=0; //是否返回了预期的数据
				ConfigModuleNoBlockCaseValue=0;
				ConfigModuleNoBlockFlage=1; break;
		}
	}
}


/**
* @brief  发送配置指令函数
* @param  order          发送数据(字符串)
* @param  FunctionSend   发送数据(运行发送数据函数)
* @param  HopeReturn1    接收数据(希望返回的字符串1)
* @param  HopeReturn2    接收数据(希望返回的字符串2)
* @param  FunctionParse  数据处理函数
* @param  ConfigFunctionValue  控制下一条数据的发送时间,取值为 ConfigFunctionCntCom:下一条立即发送
* @retval 
* @example 
**/
void SendConfigFunction(char *order,void (*FunctionSend)(),char *HopeReturn1,char *HopeReturn2,void (*FunctionParse)(char *data),u32 ConfigFunctionValue)
{
	memset(HopeReturnData1,NULL,strlen(HopeReturnData1));
	memset(HopeReturnData2,NULL,strlen(HopeReturnData2));
	if(HopeReturn1!=NULL)  sprintf(HopeReturnData1,"%s",HopeReturn1);//拷贝数据到数组 HopeReturn1,希望返回的数据1
	if(HopeReturn2!=NULL)  sprintf(HopeReturnData2,"%s",HopeReturn2);//拷贝数据到数组 HopeReturn1,希望返回的数据2
	if(FunctionSend!=NULL) FunctionSend();//调用一个函数发送指令
	
	ConfigConnectDispose = FunctionParse;//传递处理函数指针
	SendNextDelay = ConfigFunctionValue;//如果上一条数据处理成功,控制发送下一条数据的时间
	
	if(order!=NULL){
		MainLen = sprintf((char*)MainBuffer,"%s",order);//发送的数据
		UsartOutStrIT(UART4,MainBuffer,MainLen);
	}
}


/**
* @brief  处理配置返回数据:判断是否返回想要的数据
* @param  data
* @param  
* @retval 
* @example 
**/
void FunctionParseCompare(char *data)
{
	if(strlen(HopeReturnData1) != 0 && strstr(data, HopeReturnData1))
	{
		DataReturnFlage=1;
		ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt:SendNextDelay);
	}
	if(strlen(HopeReturnData2) != 0 && strstr(data, HopeReturnData2))
	{
		DataReturnFlage=1;
		ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt:SendNextDelay);
	}
}


