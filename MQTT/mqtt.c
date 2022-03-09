/**
  ******************************************************************************
  * @file    mqtt.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2020/3/19
  * @brief   
  ******************************************************************************
	一,使用说明
		1,把以下程序放在1ms定时器中断中
			mqtt_time_data(&mymqtt);
			
		2,把以下程序放到主函数
		while(1)
		{
			if(模块连接上了MQTT服务器(TCP服务器))
			{
				mqtt_send_function(&mymqtt);//提取发送缓存的MQTT协议
				mqtt_keep_alive(&mymqtt);//处理发送心跳包
				
				if(服务器返回了数据)
				{
					mqtt_read_function(&mymqtt,数据地址,数据长度);
				}
			}

		}
			
  ******************************************************************************
  */


#define MQTT_C_
#include <string.h>
#include "stdio.h"
#include "mqtt.h"
#include "mqtt_msg.h"
#include "stm32f10x.h"
#include "BufferManage.h"


//替换自己的发送函数:ctrl+F   搜索:   Send function
#include "usart.h" //网络模块采用串口通信

mqtt_t mymqtt;


/**
* @brief   发送MQTT数据:把发送数据给网络模块的函数放在此处
* @param   mqtt:  结构体指针   默认填写  &mymqtt
* @param   mqtt   发送的数据
* @param   None
* @retval  None
* @warning None
* @example
**/
void mqtt_send_function(mqtt_t *mqtt)
{
	/*可以发送数据*/
	if(mqtt->timer_out_cnt<=0 && mqtt->timer_out_send<=0)
	{
		BufferManageRead(&mqtt->buff_manage_struct_t,mqtt->send_buff,&mqtt->buff_manage_struct_t.SendLen);/*提取缓存区的数据*/
		
		if(mqtt->buff_manage_struct_t.SendLen>0)//有数据需要发送
		{
			if(mqtt_get_type(mqtt->send_buff) == MQTT_MSG_TYPE_SUBSCRIBE){//发送的消息是订阅
				mqtt->mqtt_message_id = mqtt_get_id(mqtt->send_buff, mqtt->buff_manage_struct_t.SendLen);//获取消息ID
				mqtt->mqtt_message_type = MQTT_MSG_TYPE_SUBSCRIBE;
				mqtt->timer_out_cnt = mqtt_timerout_default;//设置超时时间
			}
			else if(mqtt_get_type(mqtt->send_buff) == MQTT_MSG_TYPE_PUBLISH){//发送的消息是发布
				if(mqtt_get_qos(mqtt->send_buff) == 1 || mqtt_get_qos(mqtt->send_buff) == 2)//消息等级是1或者2
				{
					mqtt->mqtt_message_id = mqtt_get_id(mqtt->send_buff, mqtt->buff_manage_struct_t.SendLen);//获取消息ID,以便应答
					mqtt->mqtt_message_type = MQTT_MSG_TYPE_PUBLISH;
					mqtt->timer_out_cnt = mqtt_timerout_default;//设置超时时间
				}
			}
			else if(mqtt_get_type(mqtt->send_buff) == MQTT_MSG_TYPE_PUBREL){//客户端上次发送了消息等级是2的消息,服务器返回PUBREC,客户端需要返回PUBREL
				mqtt->timer_out_cnt = mqtt_timerout_default;//设置超时时间
			}
			//使用串口发送数据(发给网络模块):请替换自己的发送函数
			//mqtt->send_buff:发送的数据
			//mqtt->buff_manage_struct_t.SendLen:发送的数据长度
			//Send function

			UsartOutStrIT(UART4,mqtt->send_buff,mqtt->buff_manage_struct_t.SendLen);//请替换自己的发送函数
           
			//调用上面的发送函数以后,程序默认等待3S,再发送下一条数据!
			//解决方案1:调用自己的发送数据函数以后,在发送完成的地方写上: mymqtt.timer_out_send = 0; //记得包含 #include "mqtt.h"
			//解决方案2:如果没有发送完成的地方,请在mqtt.h里面修改  mqtt_timerout_send_default  变量的默认延时时间 ms
			
			mqtt->timer_out_send=mqtt_timerout_send_default;
		}
	}
}


/**
* @brief   接收处理MQTT数据:用户需要把网络模块接收到的MQTT数据传给此函数处理
* @param   mqtt:    结构体指针    默认填写  &mymqtt
* @param   buffer   接收的MQTT数据
* @param   length   数据长度
* @retval  None
* @warning None
* @example
**/
void mqtt_read_function(mqtt_t *mqtt,unsigned char* buffer, uint16_t length)
{
	uint8_t msg_type;
	uint8_t msg_qos;
	uint16_t msg_id;
	int DataLen=0;
	
	msg_type = mqtt_get_type(buffer);
	msg_qos = mqtt_get_qos(buffer);
	msg_id = mqtt_get_id(buffer, length);
	
	
	if(mqtt->mqtt_message_type == MQTT_MSG_TYPE_SUBSCRIBE){//上次发送的是订阅主题
		mqtt_keep_alive_init(mqtt);//初始化心跳包变量
		//上次发送的消息是订阅
		if(msg_type == MQTT_MSG_TYPE_SUBACK){//获取应答
			mqtt->timer_out_cnt = 0;//停止超时定时器
			if(msg_id == mqtt->mqtt_message_id && ( buffer[length-1]&0xff) != 0x80 )
			{
				mqtt->mqtt_message_type = MQTT_MSG_TYPE_SUBACK;
				if(mqtt->subscribedCb!=NULL){
					mqtt->subscribedCb(mqtt->mqtt_message_id);
				}
			}
			else
			{
				mqtt->mqtt_message_type = 0;
				if(mqtt->failsubscribedCb!=NULL)mqtt->failsubscribedCb(mqtt->mqtt_message_id);
			}
		}
	}
	
	switch (msg_type)
	{
		case MQTT_MSG_TYPE_PUBLISH://接收到消息
			mqtt_keep_alive_init(mqtt);//初始化心跳包变量
			if (msg_qos == 1){//消息等级是1,打包需要返回的PUBACK数据
				mqtt->mqtt_send_data_len = mqtt_msg_puback(msg_id,&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);
			}
			else if (msg_qos == 2){//消息等级是2,打包需要返回的PUBREC
				mqtt->mqtt_send_data_len = mqtt_msg_pubrec(msg_id,&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);
			}
			
			if (msg_qos == 1 || msg_qos == 2) {
					if(mqtt->mqtt_send_data_len >0 ){						
						BufferManageWrite(&mqtt->buff_manage_struct_t,mqtt->ptr,mqtt->mqtt_send_data_len,&DataLen);/*把协议存入存入缓存*/
				}
			}
			
			//调用接收回调函数
			if(mqtt->recCb){
				mqtt->topic_length = length;
				mqtt->topic = mqtt_get_publish_topic(buffer, &mqtt->topic_length);
				mqtt->data_length = length;
				mqtt->data = mqtt_get_publish_data(buffer, &mqtt->data_length);
				
				mqtt->recCb(mqtt->topic,mqtt->topic_length,mqtt->data,mqtt->data_length);
			}
			break;
			
		case MQTT_MSG_TYPE_PUBACK://客户端上次发送了消息等级是1的消息,服务器返回PUBACK,说明消息已经送达
			if(mqtt->mqtt_message_type == MQTT_MSG_TYPE_PUBLISH && mqtt->mqtt_message_id == msg_id){
				mqtt->mqtt_message_type = 0;
				mqtt->timer_out_cnt = 0;//停止超时定时器
				if(mqtt->PublishedCb){
					mqtt->PublishedCb();
				}
			}
			break;
		
		case MQTT_MSG_TYPE_PUBREC://客户端上次发送了消息等级是2的消息,服务器返回PUBREC,客户端需要返回PUBREL
			mqtt->mqtt_send_data_len = mqtt_msg_pubrel(msg_id,&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);
			mqtt->timer_out_cnt = 0;
			BufferManageWrite(&mqtt->buff_manage_struct_t,mqtt->ptr,mqtt->mqtt_send_data_len,&DataLen);/*把协议存入存入缓存*/
			break;
		case MQTT_MSG_TYPE_PUBCOMP://客户端上次发送了消息等级是2的消息,服务器返回PUBREC,客户端返回了PUBREL,服务器最后返回PUBCOMP,说明消息已经送达
			if (mqtt->mqtt_message_type == MQTT_MSG_TYPE_PUBLISH && mqtt->mqtt_message_id == msg_id) {
				mqtt->mqtt_message_type = 0;
				mqtt->timer_out_cnt = 0;//停止超时定时器
				if(mqtt->PublishedCb){
					mqtt->PublishedCb();
				}
			}
			break;
		case MQTT_MSG_TYPE_PUBREL://客户端收到了消息等级为2的消息,同时回复了PUBREC,服务器返回PUBREL,客户端最后需要返回PUBCOMP
			mqtt->mqtt_send_data_len = mqtt_msg_pubcomp(msg_id,&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);
			
			BufferManageWrite(&mqtt->buff_manage_struct_t,mqtt->ptr,mqtt->mqtt_send_data_len,&DataLen);/*把协议存入存入缓存*/
			break;
		case MQTT_MSG_TYPE_PINGRESP://接收到心跳包数据
				mqtt_keep_alive_init(mqtt);//初始化心跳包变量
			break;
		
		default:
			break;
	}
}


/**
* @brief   定时轮训函数(该函数需要放在1ms定时器)
* @param   mqtt:结构体指针 默认填写  &mymqtt
* @param   None
* @retval  None
* @warning None
* @example
**/
void mqtt_time_data(mqtt_t *mqtt)
{
	/*MQTT通信超时检测*/
	if(mqtt->timer_out_cnt>0){
		mqtt->timer_out_cnt --;
	}
	
	/*发送数据超时检测,为确保发送完一条再发送下一条*/
	if(mqtt->timer_out_send>0){
		mqtt->timer_out_send--;
	}
	
	/*MQTT心跳包*/
	if(mqtt->KeepAliveSendCount>0){
		mqtt->KeepAliveTimeOut++;
	}
	else{
		mqtt->KeepAliveTimeOut=0;
	}
	
	mqtt->KeepAliveTimeCnt++;
	if(mqtt->KeepAliveTimeCnt/1000 >= mqtt->mqtt_connect_info.keepalive)//到时间发送心跳包
	{
		mqtt->KeepAliveSendFlage = 1;
	}
}



/**
* @brief  处理发送心跳包
* @param  
* @param  None
* @param  None
* @retval None
* @example 
**/
void mqtt_keep_alive(mqtt_t *mqtt)
{

	if(mqtt->KeepAliveSendFlage)//需要发送心跳包
	{		
		mqtt_ping(mqtt);
	}
	if(mqtt->KeepAliveTimeOut >2000)//心跳包超过时间没有返回应答
	{
		mqtt->KeepAliveTimeOut = 0;
		mqtt_ping(mqtt);
	}
}


/**
* @brief  初始化心跳包变量
* @param  
* @param  None
* @param  None
* @retval None
* @example 
**/
void mqtt_keep_alive_init(mqtt_t *mqtt)
{
//	mqtt->KeepAliveTimeCnt=0;
	mqtt->KeepAliveSendCount=0;
	mqtt->KeepAliveTimeOut=0;
}

/**
* @brief   初始化MQTT结构体指针   
* @param   mqtt:结构体指针   
* @param   None
* @retval  None
* @warning None
* @example
**/
void mqtt_init(mqtt_t *mqtt)
{
	BufferManageCreate(&mqtt->buff_manage_struct_t, mqtt->send_buff_loop, send_buff_loop_len, mqtt->send_buff_loop_m, send_buff_loop_m_len*4);
	
	mqtt->mqtt_message_type=0;
	mqtt->mqtt_send_data_len=0;
	mqtt->state=0;
	mqtt->timer_out_cnt=0;
	mqtt_keep_alive_init(mqtt);
}



/**
* @brief   打包连接MQTT协议
* @param   mqtt   
* @param   data_ptr:返回打包的数据指针  
* @retval  数据个数
* @warning None
* @example
**/
int mqtt_connect(mqtt_t *mqtt,unsigned char **data_ptr)
{
	return mqtt_msg_connect(&mqtt->mqtt_connect_info,data_ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);
}



/**
* @brief  判断是否连接上MQTT
* @param  服务器返回的数据
* @param  
* @retval 0 连接成功
* @example 
**/
int  mqtt_connect_ack(unsigned char *buff)
{
	if(mqtt_get_type(buff) == MQTT_MSG_TYPE_CONNACK)
	{
		return mqtt_get_connect_ret_code(buff);
	}
	return -1;
}


/**
* @brief   注册连接函数
* @param   mqtt   
* @param   connectedCb   
* @retval  None
* @warning None
* @example
**/
void mqtt_connect_reg(mqtt_t *mqtt,MqttCallback connectCb)
{
	mqtt->connectCb = connectCb;
}

/**
* @brief   注册断开连接函数
* @param   mqtt   
* @param   connectedCb   
* @retval  None
* @warning None
* @example
**/
void mqtt_disconnect_reg(mqtt_t *mqtt,MqttCallback disconnectCb)
{
	mqtt->disconnectCb = disconnectCb;
}


/**
* @brief   注册MQTT接收到数据回调函数
* @param   mqtt   
* @param   disconnectedCb   
* @retval  None
* @warning None
* @example
**/
void mqtt_received_reg(mqtt_t *mqtt,MqttRecCallback recCb)
{
	mqtt->recCb = recCb;
}



/**
* @brief   订阅主题
* @param   mqtt:mqtt_t结构体变量   
* @param   topic:订阅的主题
* @param   qos:消息等级
* @param   subscribedCb:订阅成功回调
* @param   failsubscribedCb:订阅失败回调
* @param   None
* @retval  0:Success  1:打包数据错误  2:缓存满
* @warning None
* @example
**/
int mqtt_subscribe(mqtt_t *mqtt,unsigned  char* topic, uint8_t qos,MqttCallback1 subscribedCb,MqttCallback1 failsubscribedCb)
{
	mqtt->mqtt_send_data_len = mqtt_msg_subscribe_topic(topic,qos,&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);/*打包订阅协议*/
	mqtt->subscribedCb = subscribedCb;
	mqtt->failsubscribedCb = failsubscribedCb;
	return mqtt_into_buff(mqtt);
}


/**
* @brief   发布消息
* @param   mqtt:mqtt_t结构体变量   
* @param   topic:发布的主题
* @param   date: 发送的数据
* @param   data_length:数据长度
* @param   qos:消息等级
* @param   retain:是否需要服务器保留消息
* @param   PublishedCb:发布成功回调(只有qos=1/2时有效)
* @retval  0:Success  1:打包数据错误  2:缓存满
* @warning None
* @example
**/
int mqtt_publish(mqtt_t *mqtt,unsigned char* topic,unsigned char* date, int data_length, int qos, int retain, MqttCallback PublishedCb)
{
	mqtt->mqtt_send_data_len = mqtt_msg_publish(topic,date,data_length,qos,retain,&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);/*打包发布协议*/
	mqtt->PublishedCb = PublishedCb;
	return mqtt_into_buff(mqtt);
}


/**
* @brief   发送心跳包数据
* @param   mqtt:mqtt_t结构体变量
* @param   None
* @param   None
* @param   None
* @param   None
* @param   None
* @retval  0:Success  1:打包数据错误  2:缓存满
* @warning None
* @example
**/
int mqtt_ping(mqtt_t *mqtt)
{
	int len;
	mqtt->mqtt_send_data_len = mqtt_msg_pingreq(&mqtt->ptr,mqtt->mqtt_data_buff,mqtt_send_buff_len);/*打包协议*/
	len = mqtt_into_buff(mqtt);
	
	if(len==0){
		mqtt->KeepAliveTimeCnt=0;
		mqtt->KeepAliveSendFlage=0;
		mqtt->KeepAliveSendCount++;
		if(mqtt->KeepAliveSendCount>=4){
			mqtt->KeepAliveSendCount=0;
			if(mqtt->disconnectCb){
				mqtt->disconnectCb();
			}
		}
	}
	return len;
}



/**
* @brief   把数据插入缓存
* @param   mqtt   
* @retval  0:Success  1:打包数据错误  2:缓存满
* @warning None
* @example
**/
int mqtt_into_buff(mqtt_t *mqtt)
{
	int len;
	if(mqtt->mqtt_send_data_len >0 )
	{
		BufferManageWrite(&mqtt->buff_manage_struct_t,mqtt->ptr,mqtt->mqtt_send_data_len,&len);/*把协议存入存入缓存*/

		if(len==0){
			return 0;
		}
		else
		{
			return -2;
		}			
	}
	else
	{
		return -1;
	}
}






