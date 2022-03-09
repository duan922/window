
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


//��������ʹ��
unsigned char MainBuffer[MainBufferLen];//��������,ȫ��ͨ��
unsigned char *MainString;    //ȫ��ͨ�ñ���
u32  MainLen=0;      //ȫ��ͨ�ñ���
u32  Gas=0;
u32  bee_time;
u32  windnum=0;//���ض�ʱ
u32  windnums=0;//΢����ʱ
u32  blindnum=0;//���ض�ʱ
u32  lengh_timme=0;

char wind_flag=0;//������־λ
char wind_status=0,blind_status=0,fan_status=0,uv_status=0;//״̬��ʾ
u16 CO2,CH2O,TVOC,PM25,PM10,numb;
float TEM,HUM,voltage,CO,SMOKE;

char RelayState = 0;//��¼�̵���״̬


//����MQTT
unsigned char IP[55]="47.97.183.24";//IP��ַ/����47.97.183.24  116.62.44.118
unsigned char Port[7] = "1884";//�˿ں�
unsigned char MQTTid[50] = "";//ClientID
unsigned char MQTTUserName[20] = "admin";//�û���
unsigned char MQTTPassWord[50] = "public";//����
unsigned char MQTTkeepAlive = 30;//������ʱ��

unsigned char MQTTPublishTopic[30]="";//�洢MQTT����������
unsigned char MQTTPublishTopic1[30]="device";
unsigned char MQTTWillMessage[50] = "{\"Disconnect...\"}";//������Ϣ
unsigned char MQTTWillQos = 0;//��Ϣ�ȼ�
unsigned char MQTTWillRetained = 1;//�Ƿ���Ҫ��������¼

unsigned char MQTTSubscribeTopic[30]="";//�洢MQTT���ĵ�����
unsigned char MQTTSubscribeTopic1[30]="user";//�洢MQTT���ĵ�����
unsigned char MQTTSubscribeTopic2[30]="";//�洢MQTT���ĵ�����




void APUConfigFunction(void);//APUConfig����
void DHT11Function(void);//DHT11,��ʪ�Ȳɼ�
void RelayFunction(void);//�̵���״̬���

void GasFunction(void);//������
void safety(void);
void MqttConnect(void);//������MQTT�ص�
void MqttDisConnect(void);//MQTT�Ͽ����ӻص�
void subscribedCb(int pdata);//���ĳɹ��ص�
void failsubscribedCb(int pdata);//����ʧ�ܻص�
void PublishedCb(void);//�ɹ�������Ϣ�ص�
void MqttReceive(const char* topic, uint32_t topic_len,const char *data, uint32_t lengh);//���յ����ݻص�


int main(void)
{
  NVIC_Configuration();
	uart_init(115200,115200);	 //���ڳ�ʼ��Ϊ115200
    usart3_init(9600);
	GpioInit();
    TIM3_PWM_Init();
	DelayInit();
	Timer2_Config();
    Adc_Init();
	IWDG_Init(IWDG_Prescaler_256,255*5);
	
	/*MQTT*/
	mqtt_init(&mymqtt);
	mqtt_connect_reg(&mymqtt,MqttConnect);//ע�����ӻص�����
	mqtt_disconnect_reg(&mymqtt,MqttDisConnect);//ע��Ͽ����ӻص�����
	mqtt_received_reg(&mymqtt,MqttReceive);//ע��������ݻص�����
    light(blue);//��
    printf("start");
//   Init8266();
	while(1)
	{
		IWDG_Feed();//ι��          
		if(!SmartConfigFlage)
		{
			ConfigModuleNoBlock();//����ģ������MQTT������      
		}
		APUConfigFunction(); //APU����
		if(ConfigModuleNoBlockFlage)//(ģ��������MQTT)
		{
			mqtt_send_function(&mymqtt);//��ȡ���ͻ����MQTTЭ��
			mqtt_keep_alive(&mymqtt);//������������			
		}
		                                               
		//��ȡ���ڽ��յ�����
		BufferManageRead(&buff_manage_struct_usart4,Usart4BufferMemoryCopy,&buff_manage_struct_usart4.ReadLen);
		if(buff_manage_struct_usart4.ReadLen>0)//ģ�鷵��������
		{
			if(ConfigConnectDispose != NULL)
			{
				ConfigConnectDispose((char*)Usart4BufferMemoryCopy);//����ģ�鷵�ص�����
				memset(Usart4BufferMemoryCopy,0,buff_manage_struct_usart4.ReadLen);
			}
			
			if(ConfigModuleNoBlockFlage)//(ģ��������MQTT)
			{
				if(lengh_timme>15000)
				{			
					printf("............\r\n");
						ConfigModuleNoBlockCaseValue=0;
						ConfigModuleNoBlockFlage = 0;	
				
				}
				mqtt_read_function(&mymqtt,Usart4BufferMemoryCopy,buff_manage_struct_usart4.ReadLen);//��������MQTT����
               
			}        
		}	
		
        GasFunction();
//    
	}
}

/**
* @brief   APUConfig����
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
			KeySmartConfig[3] = 0;//�����Ժ�,ֻ�а����ɿ��ٰ��µ�ʱ��Ż����
			if(!SmartConfigFlage)//���SmartConfigFlage=0��û������
			{
				SmartConfigFlage = 1;
				APUConfig();//����
				ConfigModuleNoBlockFlage=0;
				ConfigModuleNoBlockCaseValue=0;//��������WIFI����
			}
        }
	}
}

/**
* @brief   ��ȫ���
**/
void safety(void)
{
    int vnum,gas_num1,gas_num2;
    CO2=0;CH2O=0;TVOC=0;PM25=0;
    vnum=Get_Adc(15);//pc5
    gas_num1=Get_Adc(8);//pb0
    gas_num2=Get_Adc(9);//pb1
    voltage=vnum*3/4096.00;//�ƴ���ѹ
    CO=gas_num1*3/4096.00;//CO��ѹ
    SMOKE=gas_num2*3/4096.00;//�����ѹ
    CO2=rx3buffer[2]*256+rx3buffer[3];//����CO2ֵ
    CH2O=rx3buffer[4]*256+rx3buffer[5];//����CH2Oֵ
    TVOC=rx3buffer[6]*256+rx3buffer[7];//����TVOCֵ
    PM25=rx3buffer[8]*256+rx3buffer[9];//����PM25ֵ
    PM10=rx3buffer[10]*256+rx3buffer[11];//����PM10ֵ      
    TEM=(rx3buffer[12]*10+rx3buffer[13])/10.0;//����TEMֵ
    HUM=(rx3buffer[14]*10+rx3buffer[15])/10.0;//����HUMֵ
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

void GasFunction(void)//������
{
    if(Gas>=1500)
	{
        Gas=0;
        safety(); 
        if(ConfigModuleNoBlockFlage)//����ģ��ɹ�(ģ��������MQTT)
		{
            rx3_cnt=0;
            MainLen = sprintf((char*)MainBuffer,"{\"MAC\":%s,\"CO2\":%d,\"CH2O\":%d,\"TV\":%d,\"P25\":%d,\"P10\":%d,\"TEM\":%2.1f,\"HUM\":%2.1f,\"CO\":%1.2f,\"S\":%1.2f,\"VOL\":%1.3f,\"T\":%d,\"B\":%d,\"F\":%d,\"UV\":%d}",&MQTTid[0],CO2,CH2O,TVOC,PM25,PM10,TEM,HUM,CO,SMOKE,voltage,wind_status,blind_status,fan_status,uv_status);                               
			mqtt_publish(&mymqtt,MQTTPublishTopic,MainBuffer,MainLen, 0, 0,PublishedCb);//������Ϣ
		}
         
    }
}

/**
* @brief   ������MQTT�ص�����
* @param   None
* @retval  None
* @warning None
* @example
**/
void MqttConnect()
{
	memset(MQTTSubscribeTopic,NULL,sizeof(MQTTSubscribeTopic));
	sprintf((char*)MQTTSubscribeTopic,"%s%s","user/",&MQTTid[0]);//����ַ���
	mqtt_subscribe(&mymqtt, MQTTSubscribeTopic, 0,subscribedCb,failsubscribedCb);//��������
	
	memset(MQTTSubscribeTopic,NULL,sizeof(MQTTSubscribeTopic));
	sprintf((char*)MQTTSubscribeTopic2,"%s%s","device/",&MQTTid[0]);//����ַ���
	mqtt_subscribe(&mymqtt, MQTTSubscribeTopic2, 0,subscribedCb,failsubscribedCb);//��������
}

/**
* @brief   MQTT�Ͽ����ӻص�
* @param   None
* @retval  None
* @warning None
* @example
**/ 
void MqttDisConnect()
{
	mqtt_init(&mymqtt);	
	//��������ģ������MQTT
	ConfigModuleNoBlockCaseValue=0;
	ConfigModuleNoBlockFlage = 0;	
	light(blue);
	printf("\r\n���ӶϿ�**********************************************************\r\n");
}


/**
* @brief   MQTT�������ݻص�
* @param   topic:����
* @param   topic_len:���ⳤ��
* @param   data:���յ�����
* @param   lengh:���յ����ݳ���
* @retval  None
* @warning None
* @example
**/
void MqttReceive(const char* topic, uint32_t topic_len,const char *data, uint32_t lengh)
{	
	lengh_timme=0;
	if(strstr((char*)data, "\"data\":\"switch\""))//ѯ�ʿ���user/8c:aa:b5:71:61:ff  device/8c:aa:b5:71:61:ff device/e0:98:06:0c:02:18{"data":"switch","bit":"1","status":"1"}
	{
        /*�����ƴ�����*/
		if(strstr((char*)data, "\"bit\":\"1\""))//��һ·����
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//��
			{
                if(wind_flag!=1)
                {
                    windnum=0;
                    windows(fanz);
                    wind_status=0;
                    wind_flag=1;
                }
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//��
			{
                if(wind_flag!=1)
                {
                    windnum=0;
                    windows(zhengz);
                    wind_status=1;
                    wind_flag=1;
                }
			}
            else if(strstr((char*)data, "\"status\":\"-1\""))//ͣ
			{              
                    windnums=0;
                    windows(fanz);
                    wind_status=0;
                    wind_flag=2;
                   
			}
			
		}
        /*���ư�Ҷ������*/
        if(strstr((char*)data, "\"bit\":\"2\""))//�ڶ�·����//ѯ�ʿ���{"data":"switch","bit":"2","status":"1"}
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//��
			{
                blindnum=0;
				blind_control(zhengz);
                blind_status=0;
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//��
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
        /*���Ʒ��ȿ���*/
        if(strstr((char*)data, "\"bit\":\"3\""))//����·����ѯ�ʿ���{"data":"switch","bit":"3","status":"1"}
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//��
			{
				fan_control(stop);
				fan_status=0;
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//��
			{
				fan_control(zhengz);
				fan_status=1;
			}
		
		}
        /*���������*/
        if(strstr((char*)data, "\"bit\":\"4\""))//����·����ѯ�ʿ���{"data":"switch","bit":"4","status":"1"}
		{
		  if( strstr((char*)data, "\"status\":\"0\"") )//��
			{
				light_control(stop);
				uv_status=0;
			}
			else if(strstr((char*)data, "\"status\":\"1\""))//��
			{
				light_control(zhengz);
				uv_status=1;
			}		
		}
	}
	
}


/**
* @brief   ��������ɹ�
* @param   None
* @retval  None
* @warning None
* @example
**/
void subscribedCb(int pdata)
{
	printf("\r\n�ɹ���������\r\n");
}

/**
* @brief   ��������ʧ��
* @param   None
* @retval  None
* @warning None
* @example
**/
void failsubscribedCb(int pdata)
{
	printf("\r\n��������ʧ��\r\n");
}


/**
* @brief   �����ɹ�
* @param   None
* @retval  None
* @warning None
* @example
**/
void PublishedCb()
{
	printf("\r\n�����ɹ�\r\n");
}



