#ifndef __USART_H
#define __USART_H
#include <stm32f10x.h>
#ifndef USART_C_//���û�ж���
#define USART_Ex_ extern
#else
#define USART_Ex_
#endif


#include "BufferManage.h"
#include "LoopList.h"

/****************************����1****************************/
/*
����1��������:�������
����1��������:���ζ���+�ж�
*/
USART_Ex_ buff_manage_struct buff_manage_struct_usart1;//���ڽ��ջ������
USART_Ex_ rb_t rb_t_usart1;//����1���ͻ��滷�ζ���

#define Usart1ReadBuffLen 512  //����1�������������С
#define Usart1BufferMemoryLen 1024 //���ڽ��ջ�������С
#define Usart1BufferMemoryReadLen 512 //��ȡ�����������ݵ������С
#define Usart1BufferMemoryManageLen 50 //���ڽ��ջ���������
#define Usart1SendBufflen 1024  //����1�������ݻ����С

USART_Ex_ unsigned char Usart1ReadBuff[Usart1ReadBuffLen];  //�������ݻ���
USART_Ex_ u32  Usart1ReadCnt;//����1���յ������ݸ���

USART_Ex_ u8   Usart1BufferMemory[Usart1BufferMemoryLen];//�������ݻ�����
USART_Ex_ u8   Usart1BufferMemoryRead[Usart1BufferMemoryReadLen];//��ȡ�����������ݵ�����
USART_Ex_ u32  Usart1BufferMemoryManage[Usart1BufferMemoryManageLen];//��������

USART_Ex_ unsigned char Usart1SendBuff[Usart1SendBufflen];  //����1�������ݻ���

/****************************����2****************************/
/*
����2��������:�������
����2��������:��ͨ�ж�
*/
USART_Ex_ buff_manage_struct buff_manage_struct_usart4;//���ڽ��ջ������

#define Usart4ReadBuffLen 512 //�������ݻ���
#define Usart4BufferMemoryLen 1024 //�������ݻ�����
#define Usart4BufferMemoryCopyLen 512 //��ȡ������������
#define Usart4BufferMemoryManageLen 50 //��������

USART_Ex_ unsigned char Usart4ReadBuff[Usart4ReadBuffLen];  //�������ݻ���
USART_Ex_ u32  Usart4ReadCnt;//���ڽ��յ������ݸ���

USART_Ex_ u8   Usart4BufferMemory[Usart4BufferMemoryLen];//�������ݻ�����
USART_Ex_ u8   Usart4BufferMemoryCopy[Usart4BufferMemoryCopyLen];//��ȡ������������
USART_Ex_ u32  Usart4BufferMemoryManage[Usart4BufferMemoryManageLen];//��������

USART_Ex_ unsigned char *Usart4SendData;//�����жϷ������ݵĵ�ַ
USART_Ex_ u32   Usart4SendDataCnt;//�����жϷ������ݵĸ���


void UsartOutStrIT(USART_TypeDef* USARTx, unsigned char *c,uint32_t cnt);
void uart_init(u32 bound1,u32 bound2);//���ڳ�ʼ��
void UsartOutStr(USART_TypeDef* USARTx, unsigned char *c,uint32_t cnt);//���ڷ����ַ�������
void UsartOutChar(USART_TypeDef* USARTx, unsigned char c);//���ڷ���һ���ֽ�
#endif


