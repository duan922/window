#ifndef CONFIG8266_H_
#define CONFIG8266_H_

#ifndef CONFIG8266_C_//���û�ж���  _TIME_C_
#define CONFIG8266_EX_ extern
#else
#define CONFIG8266_EX_
#endif

#include <stm32f10x.h>
#include "System.h"

#define Rst8266PinOut PAout(15)       //��λ8266������
#define SmartConfigPinOut PEout(5)  //SmartConfigָʾ������c13
CONFIG8266_EX_ char ThisMAC[18];//��¼�豸MAC
CONFIG8266_EX_ char SmartConfigFlage;//�ǲ���������
CONFIG8266_EX_ char SmartConfigSuccess;//�Ƿ������ɹ�
CONFIG8266_EX_ u32  SmartConfigCnt;//��������·������ʱʹ��
CONFIG8266_EX_ u32  Config8266LedDelay;//����8266ָʾ����ҫ

char AutoConnectTCP(void);//����TCP������
char APUConfig(void);
void Init8266(void);//����·����
void Rst8266(void);//��λ8266
#endif

