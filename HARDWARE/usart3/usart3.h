#ifndef _USART3_H
#define _USART3_H
#include "stdio.h"
#include "System.h"
extern u32 rx3_cnt;
extern u8 rx3buffer[17];
void  usart3_init(u32 bound);
void  USART3_Send (u8 Data);
void  USART3_Send_String(u8 *Data);
unsigned char uart3_recByte(void) ;



#endif

