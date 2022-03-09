#ifndef __MAIN_H_
#define __MAIN_H_
#include <stm32f10x.h>

#ifndef __MAIN_C_
#define __MAIN_EXT_ extern
#else
#define __MAIN_EXT_
#endif


//缓存数据使用
#define MainBufferLen 200
__MAIN_EXT_ unsigned char MainBuffer[MainBufferLen];//缓存数据,全局通用
__MAIN_EXT_ u32  MainLen;      //全局通用变量
__MAIN_EXT_ u32  Gas;      //全局通用变量
__MAIN_EXT_ u32  bee_time;
__MAIN_EXT_ u32  windnum;      //全局通用变量
__MAIN_EXT_ u32  windnums;      //全局通用变量
__MAIN_EXT_ u32  blindnum;
__MAIN_EXT_ u32 lengh_timme;
__MAIN_EXT_ char  wind_flag;      //全局通用变量

__MAIN_EXT_ unsigned char *MainString;    //全局通用变量



__MAIN_EXT_ unsigned char IP[55];//IP地址/域名
__MAIN_EXT_ unsigned char Port[7];//端口号
__MAIN_EXT_ unsigned char MQTTid[50];//ClientID
__MAIN_EXT_ unsigned char MQTTUserName[20];//用户名
__MAIN_EXT_ unsigned char MQTTPassWord[50];//密码
__MAIN_EXT_ unsigned char MQTTkeepAlive;//心跳包时间
__MAIN_EXT_ unsigned char MQTTPublishTopic[30];//存储MQTT发布的主题
__MAIN_EXT_ unsigned char MQTTWillMessage[50];//遗嘱消息
__MAIN_EXT_ unsigned char MQTTWillQos;//消息等级
__MAIN_EXT_ unsigned char MQTTWillRetained;//是否需要服务器记录



#endif
