/**
  ******************************************************************************
  * @author  yang feng wu 
  * @version V1.0.0
  * @date    2019/12/15
  * @brief   
  ******************************************************************************
	
  ******************************************************************************
  */

#define MQTTCLIENT_C_//如果没有定义

#include "mqtt_msg.h"
#include "string.h"
#include "stm32f10x.h"


#define MQTT_MAX_FIXED_HEADER_SIZE 3

uint16_t mqtt_message_id = 0;

enum mqtt_connect_flag
{
  MQTT_CONNECT_FLAG_USERNAME = 1 << 7,
  MQTT_CONNECT_FLAG_PASSWORD = 1 << 6,
  MQTT_CONNECT_FLAG_WILL_RETAIN = 1 << 5,
  MQTT_CONNECT_FLAG_WILL = 1 << 2,
  MQTT_CONNECT_FLAG_CLEAN_SESSION = 1 << 1
};
//__attribute((__packed__))
struct  mqtt_connect_variable_header
{
  uint8_t lengthMsb;
  uint8_t lengthLsb;
  uint8_t magic[4];
  uint8_t version;
  uint8_t flags;
  uint8_t keepaliveMsb;
  uint8_t keepaliveLsb;
};


int mqtt_get_type(unsigned char* buffer) { return (buffer[0] & 0xf0) >> 4; }
int mqtt_get_connect_ret_code(unsigned char* buffer) { return (buffer[3]); }
int mqtt_get_qos(unsigned char* buffer) { return (buffer[0] & 0x06) >> 1; }


int append_string(int *length,unsigned  char* buffer,int buffer_length,unsigned  char* string, int len)
{
  if((*length) + len + 2 > buffer_length)//加上 ClientID 和 记录 ClientID个数(两位) 以后超出了数组
    return -1;

  buffer[(*length)++] = len >> 8;
  buffer[(*length)++] = len & 0xff;
  c_memcpy(buffer + (*length), string, len);
  (*length) += len;
  return len + 2;
}



uint16_t append_message_id(int *length,unsigned  char* buffer,int buffer_length, uint16_t message_id)
{
  // If message_id is zero then we should assign one, otherwise
  // we'll use the one supplied by the caller
  while(message_id == 0)
    message_id = ++mqtt_message_id;

  if((*length) + 2 > buffer_length)
    return 0;

  buffer[(*length)++] = message_id >> 8;
  buffer[(*length)++] = message_id & 0xff;
	
  return message_id;
}


int fini_message(unsigned char **data_ptr,int	length,unsigned char* buffer, int type, int dup, int qos, int retain)
{
  int remaining_length = length - MQTT_MAX_FIXED_HEADER_SIZE;
	
  if(remaining_length > 127)
  {
    buffer[0] = ((type & 0x0f) << 4) | ((dup & 1) << 3) | ((qos & 3) << 1) | (retain & 1);
    buffer[1] = 0x80 | (remaining_length % 128);
    buffer[2] = remaining_length / 128;
    length = remaining_length + 3;
    *data_ptr = buffer;
  }
  else
  {
    buffer[1] = ((type & 0x0f) << 4) | ((dup & 1) << 3) | ((qos & 3) << 1) | (retain & 1);
    buffer[2] = remaining_length;
    length = remaining_length + 2;
    *data_ptr = buffer + 1;
  }

  return length;
}



uint16_t mqtt_get_id(unsigned char* buffer, uint16_t length)
{
  if(length < 1)
    return 0;
	
  switch(mqtt_get_type(buffer))
  {
    case MQTT_MSG_TYPE_PUBLISH:
    {
      int i;
      int topiclen;

      for(i = 1; i < length; ++i)
      {
        if((buffer[i] & 0x80) == 0)
        {
          ++i;
          break;
        }
      }

      if(i + 2 >= length)
        return 0;
      topiclen = buffer[i++] << 8;
      topiclen |= buffer[i++];

      if(i + topiclen >= length)
        return 0;
      i += topiclen;

      if(mqtt_get_qos(buffer) > 0)
      {
        if(i + 2 >= length)
          return 0;
        //i += 2;
      } else {
    	  return 0;
      }
      return (buffer[i] << 8) | buffer[i + 1];
    }
    case MQTT_MSG_TYPE_PUBACK:
    case MQTT_MSG_TYPE_PUBREC:
    case MQTT_MSG_TYPE_PUBREL:
    case MQTT_MSG_TYPE_PUBCOMP:
    case MQTT_MSG_TYPE_SUBACK:
    case MQTT_MSG_TYPE_UNSUBACK:
    case MQTT_MSG_TYPE_SUBSCRIBE:
    {
      // This requires the remaining length to be encoded in 1 byte,
      // which it should be.
      if(length >= 4 && (buffer[1] & 0x80) == 0)
        return (buffer[2] << 8) | buffer[3];
      else
        return 0;
    }

    default:
      return 0;
  }
}



/**
* @brief   获取MQTT返回的数据长度(去掉1和2字节后面数据的长度)
* @param   buffer   MQTT返回的数据首地址
* @param   length   返回的数据个数
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_get_total_length(unsigned char* buffer, uint16_t length)
{
  int i;
  int totlen = 0;

  for(i = 1; i < length; ++i)
  {
    totlen += (buffer[i] & 0x7f) << (7 * (i - 1));
    if((buffer[i] & 0x80) == 0)
    {
      ++i;
      break;
    }
  }
  totlen += i;

  return totlen;
}




/**
* @brief   打包连接MQTT指令
* @param   info     MQTT信息
* @param   data_ptr 打包的数据首地址
* @param   buffer   打包进的数组
* @param   buffer_length 数组长度
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_msg_connect(mqtt_connect_info_t* info,unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
  struct mqtt_connect_variable_header* variable_header;
	
	mqtt_message_id = 0;
	
	length = MQTT_MAX_FIXED_HEADER_SIZE;//头.连接类型1位,数据个数2位(如果大于127就需要两位)
	
  if(length + sizeof(*variable_header) > buffer_length)//数组不够存储的
    return 0;
	
  variable_header = (void*)(buffer + length);//把数组分给这个结构体里面的变量
  length += sizeof(*variable_header);//存储完 连接类型,整个数据个数,版本号个数,版本号,等
	
  variable_header->lengthMsb = 0;//版本名称个数高位
  variable_header->lengthLsb = 4;//版本名称个数低位
  c_memcpy(variable_header->magic, "MQTT", 4);//版本名称MQTT
  variable_header->version = 4;//版本号
  variable_header->flags = 0;//先清零
  variable_header->keepaliveMsb = info->keepalive >> 8;//心跳包时间
  variable_header->keepaliveLsb = info->keepalive & 0xff;//心跳包时间

  if(info->clean_session)//清除连接信息
    variable_header->flags |= MQTT_CONNECT_FLAG_CLEAN_SESSION;

  if(info->client_id != NULL && info->client_id[0] != '\0')//client_id
  {
    if(append_string(&length,buffer,buffer_length, info->client_id, c_strlen(info->client_id)) < 0)//拷贝
      return -1;//数组不够用呀...
  }
  else
    return -2;//没有设置client_id

  if(info->will_topic != NULL && info->will_topic[0] != '\0')//遗嘱
  {
    if(append_string(&length,buffer,buffer_length , info->will_topic, c_strlen(info->will_topic)) < 0)//遗嘱的主题
      return -3;

    if(append_string(&length,buffer,buffer_length , info->will_message, c_strlen(info->will_message)) < 0)//遗嘱的消息
      return -4;

    variable_header->flags |= MQTT_CONNECT_FLAG_WILL;//需要遗嘱
    if(info->will_retain)//遗嘱是够需要服务器保留
      variable_header->flags |= MQTT_CONNECT_FLAG_WILL_RETAIN;//保留遗嘱
    variable_header->flags |= (info->will_qos & 3) << 3;//遗嘱消息等级
  }
	
  if(info->username != NULL && info->username[0] != '\0')//username
  {
    if(append_string(&length,buffer,buffer_length, info->username, c_strlen(info->username)) < 0)//拷贝用户名
      return -5;

    variable_header->flags |= MQTT_CONNECT_FLAG_USERNAME;//有用户名
  }
	
  if(info->password != NULL && info->password[0] != '\0')//password
  {
    if(append_string(&length,buffer,buffer_length, info->password, c_strlen(info->password)) < 0)
      return -6;

    variable_header->flags |= MQTT_CONNECT_FLAG_PASSWORD;//有密码
  }

  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_CONNECT, 0, 0, 0);//最终组合连接MQTT的指令
}


/**
* @brief  判断是否连接上MQTT
* @param  服务器返回的数据
* @param  
* @retval 0 连接成功
* @example 
**/
int  mqtt_msg_connect_ack(unsigned char *buff)
{
	if(mqtt_get_type(buff) == MQTT_MSG_TYPE_CONNACK)
	{
		return mqtt_get_connect_ret_code(buff);
	}
	return -1;
}


/**
* @brief   断开连接
* @param   data_ptr 打包的数据首地址
* @param   buffer   打包进的数组
* @param   buffer_length 数组长度
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_msg_disconnect(unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
	length = MQTT_MAX_FIXED_HEADER_SIZE;
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_DISCONNECT, 0, 0, 0);
}



/**
* @brief   订阅主题
* @param   topic   订阅的主题
* @param   qos     消息等级
* @param   data_ptr 打包的数据首地址
* @param   buffer   打包进的数组
* @param   buffer_length 数组长度
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_msg_subscribe_topic(unsigned char* topic, int qos,unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
	length = MQTT_MAX_FIXED_HEADER_SIZE;
	
	if(topic == NULL || topic[0] == '\0')
		return -1;
	
	if((mqtt_message_id = append_message_id(&length, buffer, buffer_length, 0)) == 0)
		return -2;
	
	if(append_string(&length, buffer, buffer_length, topic, c_strlen(topic)) < 0)
		return -3;
	
	if(length + 1 > buffer_length)
    return -4;
  buffer[length++] = qos;
	
	return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_SUBSCRIBE, 0, 1, 0);
}



/**
* @brief  判断是否成功订阅
* @param  buffer  服务器返回的数据
* @param  length  服务器返回的数据长度
* @retval 0:成功  1:失败
* @example 
**/
int mqtt_msg_subscribe_ack(unsigned char* buffer, uint16_t length)
{
	if(mqtt_get_type(buffer) == MQTT_MSG_TYPE_SUBACK)
	{
		if(mqtt_get_id(buffer,length) == mqtt_message_id)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
}


/**
* @brief   发布消息
* @param   topic    主题
* @param   data     消息
* @param   data_length 消息长度
* @param   qos      消息等级
* @param   retain   是否需要保留消息
* @param   data_ptr 打包的数据首地址
* @param   buffer   打包进的数组
* @param   buffer_length 数组长度
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_msg_publish(unsigned char* topic,unsigned  char* date, int data_length, int qos, int retain,unsigned  char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
	length = MQTT_MAX_FIXED_HEADER_SIZE;

  if(topic == NULL || topic[0] == '\0')
    return -1;

  if(append_string(&length, buffer, buffer_length, topic, strlen(topic)) < 0)
    return -2;

  if(qos > 0)
  {
    if((mqtt_message_id = append_message_id(&length, buffer, buffer_length,  0)) == 0)
      return -3;
  }
  else
    mqtt_message_id = 0;

  if(length + data_length > buffer_length)
    return -4;
  memcpy(buffer + length, date, data_length);
  length += data_length;

  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PUBLISH, 0, qos, retain);
}



int mqtt_msg_puback(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
  length = MQTT_MAX_FIXED_HEADER_SIZE;
  if(append_message_id(&length, buffer, buffer_length,message_id) == 0)
    return -1;
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PUBACK, 0, 0, 0);
}


int mqtt_msg_pubrec(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
  length = MQTT_MAX_FIXED_HEADER_SIZE;
  if(append_message_id(&length, buffer, buffer_length,message_id) == 0)
    return -1;
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PUBREC, 0, 0, 0);
}


int mqtt_msg_pubrel(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
  length = MQTT_MAX_FIXED_HEADER_SIZE;
	
  if(append_message_id(&length, buffer, buffer_length,message_id) == 0)
    return -1;
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PUBREL, 0, 1, 0);
}


int mqtt_msg_pubcomp(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
  length = MQTT_MAX_FIXED_HEADER_SIZE;
  if(append_message_id(&length, buffer, buffer_length,message_id) == 0)
    return -1;
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PUBCOMP, 0, 0, 0);
}


const char* mqtt_get_publish_topic(unsigned char* buffer, uint16_t* length)
{
  int i;
  int totlen = 0;
  int topiclen;

  for(i = 1; i < *length; ++i)
  {
    totlen += (buffer[i] & 0x7f) << (7 * (i -1));
    if((buffer[i] & 0x80) == 0)
    {
      ++i;
      break;
    }
  }
  totlen += i;

  if(i + 2 >= *length)
    return NULL;
  topiclen = buffer[i++] << 8;
  topiclen |= buffer[i++];

  if(i + topiclen > *length)
    return NULL;

  *length = topiclen;
  return (const char*)(buffer + i);
}


const char* mqtt_get_publish_data(unsigned char* buffer, uint16_t* length)
{
  int i;
  int totlen = 0;
  int topiclen;
  int blength = *length;
  *length = 0;

  for(i = 1; i < blength; ++i)
  {
    totlen += (buffer[i] & 0x7f) << (7 * (i - 1));
    if((buffer[i] & 0x80) == 0)
    {
      ++i;
      break;
    }
  }
  totlen += i;

  if(i + 2 >= blength)
    return NULL;
  topiclen = buffer[i++] << 8;
  topiclen |= buffer[i++];

  if(i + topiclen >= blength)
    return NULL;

  i += topiclen;

  if(mqtt_get_qos(buffer) > 0)
  {
    if(i + 2 >= blength)
      return NULL;
    i += 2;
  }

  if(totlen < i)
    return NULL;

  if(totlen <= blength)
    *length = totlen - i;
  else
    *length = blength - i;
  return (const char*)(buffer + i);
}

/**
* @brief   打包服务器返回的心跳包数据(用不到)
* @param   data_ptr 打包的数据首地址
* @param   buffer   打包进的数组
* @param   buffer_length 数组长度
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_msg_pingresp(unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
	length = MQTT_MAX_FIXED_HEADER_SIZE;	
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PINGRESP, 0, 0, 0);
}

/**
* @brief   获取发送给服务器的心跳包数据
* @param   data_ptr 打包的数据首地址
* @param   buffer   打包进的数组
* @param   buffer_length 数组长度
* @retval  数据长度 
* @warning None
* @example 
**/
int mqtt_msg_pingreq(unsigned char **data_ptr,unsigned char* buffer,int buffer_length)
{
	int length;
	length = MQTT_MAX_FIXED_HEADER_SIZE;	
  return fini_message(data_ptr,length, buffer, MQTT_MSG_TYPE_PINGREQ, 0, 0, 0);
}


