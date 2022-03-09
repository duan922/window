#ifndef MQTTCLIENT_H_
#define MQTTCLIENT_H_


#ifndef MQTTCLIENT_C_//如果没有定义
#define MQTTCLIENT_Cx_ extern
#else
#define MQTTCLIENT_Cx_
#endif

#include "string.h"
#include "stm32f10x.h"

#define c_memcpy memcpy
#define c_memset memset
#define c_strlen strlen


enum mqtt_message_type
{
  MQTT_MSG_TYPE_CONNECT = 1,
  MQTT_MSG_TYPE_CONNACK = 2,
  MQTT_MSG_TYPE_PUBLISH = 3,
  MQTT_MSG_TYPE_PUBACK = 4,
  MQTT_MSG_TYPE_PUBREC = 5,
  MQTT_MSG_TYPE_PUBREL = 6,
  MQTT_MSG_TYPE_PUBCOMP = 7,
  MQTT_MSG_TYPE_SUBSCRIBE = 8,
  MQTT_MSG_TYPE_SUBACK = 9,
  MQTT_MSG_TYPE_UNSUBSCRIBE = 10,
  MQTT_MSG_TYPE_UNSUBACK = 11,
  MQTT_MSG_TYPE_PINGREQ = 12,
  MQTT_MSG_TYPE_PINGRESP = 13,
  MQTT_MSG_TYPE_DISCONNECT = 14
};

enum mqtt_connack_return_code
{
    MQTT_CONN_FAIL_SERVER_NOT_FOUND = -5,
    MQTT_CONN_FAIL_NOT_A_CONNACK_MSG = -4,
    MQTT_CONN_FAIL_DNS = -3,
    MQTT_CONN_FAIL_TIMEOUT_RECEIVING = -2,
    MQTT_CONN_FAIL_TIMEOUT_SENDING = -1,
    MQTT_CONNACK_ACCEPTED = 0,
    MQTT_CONNACK_REFUSED_PROTOCOL_VER = 1,
    MQTT_CONNACK_REFUSED_ID_REJECTED = 2,
    MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE = 3,
    MQTT_CONNACK_REFUSED_BAD_USER_OR_PASS = 4,
    MQTT_CONNACK_REFUSED_NOT_AUTHORIZED = 5
};



//连接MQTT指令 
typedef struct mqtt_connect_info
{
  unsigned  char* client_id;
  unsigned  char* username;
  unsigned  char* password;
  unsigned  char* will_topic;
  unsigned  char* will_message;
  int keepalive;
  int will_qos;
  int will_retain;
  int clean_session;
	
} mqtt_connect_info_t;


int mqtt_get_type(unsigned char* buffer);
int mqtt_get_connect_ret_code(unsigned char* buffer);
int mqtt_get_qos(unsigned char* buffer);
uint16_t mqtt_get_id(unsigned char* buffer, uint16_t length);

int mqtt_msg_connect(mqtt_connect_info_t* info,unsigned char **data_ptr,unsigned char* buffer,int buffer_length);
int mqtt_msg_connect_ack(unsigned char *buff);
int mqtt_msg_subscribe_topic(unsigned char* topic, int qos,unsigned char **data_ptr,unsigned char* buffer,int buffer_length);
int mqtt_msg_subscribe_ack(unsigned char* buffer, uint16_t length);
int mqtt_msg_publish(unsigned char* topic,unsigned char* date, int data_length, int qos, int retain,unsigned  char **data_ptr,unsigned char* buffer,int buffer_length);

int mqtt_get_total_length(unsigned char* buffer, uint16_t length);

int mqtt_msg_puback(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length);
int mqtt_msg_pubrel(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length);
int mqtt_msg_pubrec(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length);
int mqtt_msg_pubcomp(uint16_t message_id,unsigned char **data_ptr,unsigned char* buffer,int buffer_length);

const char* mqtt_get_publish_topic(unsigned char* buffer, uint16_t* length);
const char* mqtt_get_publish_data(unsigned char* buffer, uint16_t* length);

int mqtt_msg_pingreq(unsigned char **data_ptr,unsigned char* buffer,int buffer_length);

#endif



