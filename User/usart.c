/**
  ******************************************************************************
  * @file    usart.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   �������ú���
				
  ******************************************************************************
  */
#define USART_C_
#include <stdio.h>
#include "usart.h"
#include "mqtt.h"
#include "BufferManage.h"


/****************************����1****************************/
unsigned char Usart1ReadBuff[Usart1ReadBuffLen]={0};  //�������ݻ���
u32  Usart1ReadCnt = 0;//���ڽ��յ������ݸ���

u8   Usart1BufferMemory[Usart1BufferMemoryLen];//�������ݻ�����
u8   Usart1BufferMemoryRead[Usart1BufferMemoryReadLen];//��ȡ������������
u32  Usart1BufferMemoryManage[Usart1BufferMemoryManageLen];//��������

unsigned char Usart1SendBuff[Usart1SendBufflen];  //���ڷ������ݻ���
u8 Usart1SendBits=0;//������ȡ���ζ���1���ֽ�

unsigned char *Usart1SendData;//�����жϷ������ݵĵ�ַ
u32   Usart1SendDataCnt=0;//�����жϷ������ݵĸ���


/****************************����4****************************/
unsigned char Usart4ReadBuff[Usart4ReadBuffLen]={0};  //�������ݻ���
u32  Usart4ReadCnt = 0;//���ڽ��յ������ݸ���

u8   Usart4BufferMemory[Usart4BufferMemoryLen];//�������ݻ�����
u8   Usart4BufferMemoryCopy[Usart4BufferMemoryCopyLen];//��ȡ������������
u32  Usart4BufferMemoryManage[Usart4BufferMemoryManageLen];//��������

unsigned char *Usart4SendData;//�����жϷ������ݵĵ�ַ
u32   Usart4SendDataCnt=0;//�����жϷ������ݵĸ���



/**
* @brief   ����1��ʼ��
* @param   bound  ������
* @param   None
* @param   None
* @retval  None
* @warning None
* @example 
**/
void uart_init(u32 bound1,u32 bound2){
  //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	rbCreate(&rb_t_usart1,Usart1SendBuff,Usart1SendBufflen);//�������ζ���(���ڷ�������)
	BufferManageCreate(&buff_manage_struct_usart1,Usart1BufferMemory,Usart1BufferMemoryLen,Usart1BufferMemoryManage,Usart1BufferMemoryManageLen*4);/*���ڽ��ջ���*/
	
	BufferManageCreate(&buff_manage_struct_usart4,Usart4BufferMemory,Usart4BufferMemoryLen,Usart4BufferMemoryManage,Usart4BufferMemoryManageLen*4);/*���ڽ��ջ���*/
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;/*����--1*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;/*����--2*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
    
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3|RCC_APB1Periph_UART4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC|RCC_APB2Periph_USART1 , ENABLE);	//??USART1,GPIOA??
    
	 //USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
  
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10 
	
  //USART2_TX   GPIOC.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //Pc10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  //USART2_RX	  GPIOC.11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//Pc11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
	
  //USART ��ʼ������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	
	USART_InitStructure.USART_BaudRate = bound1;//���ڲ�����
  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_InitStructure.USART_BaudRate = bound2;//���ڲ�����
	USART_Init(UART4, &USART_InitStructure); //��ʼ������4
	
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�����ж�
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//�����ж�
	
	
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���
	USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ���
}


/**
* @brief  �����жϷ�������
* @param  c:���ݵ��׵�ַ  cnt:���͵����ݸ���
* @param  None
* @param  None
* @retval None
* @example 
**/
void UsartOutStrIT(USART_TypeDef* USARTx, unsigned char *c, uint32_t cnt)
{
	if(USARTx == USART1){
		PutData(&rb_t_usart1,c,cnt);
		USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
	}
	else if(USARTx == UART4){
		Usart4SendData=c;
		Usart4SendDataCnt=cnt;
		USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
		
		PutData(&rb_t_usart1,"\r\n>>����:\r\n",11);
		PutData(&rb_t_usart1,c,cnt);
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
}


/**
* @brief  ���ڷ����ַ�������
* @param  *c:���͵�����ָ��  cnt:���ݸ���
* @param  None
* @param  None
* @retval None
* @example 
**/
void UsartOutStr(USART_TypeDef* USARTx, unsigned char *c,uint32_t cnt)
{
	while(cnt--)
	{
		USART_SendData(USARTx, *c++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
	}
}


/**
* @brief  ���ڷ���һ���ֽ�
* @param  *c:���͵�����ָ��  cnt:���ݸ���
* @param  None
* @param  None
* @retval None
* @example 
**/
void UsartOutChar(USART_TypeDef* USARTx, unsigned char c)
{
	USART_SendData(USARTx,c);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
}



void USART1_IRQHandler(void)//����1�жϷ������
{
	u8 Res;
	int value;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
		Usart1ReadBuff[Usart1ReadCnt] = Res;	//���յ����ݴ�������
		Usart1ReadCnt++;
		if(Usart1ReadCnt > Usart1ReadBuffLen -10)//��ֹ�������
		{
			Usart1ReadCnt = 0;
		}
	} 
	else if(USART_GetITStatus(USART1,USART_IT_IDLE) == SET)//�����ж�
	{
		USART1->DR; //���USART_IT_IDLE��־
		//�����ݴ��뻺��
	  BufferManageWrite(&buff_manage_struct_usart1,Usart1ReadBuff,Usart1ReadCnt,&value);
		Usart1ReadCnt=0;
  }  
	
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
  {
    if(rbCanRead(&rb_t_usart1)>0)//�����������ݸ�������0
    {
			rbRead(&rb_t_usart1,&Usart1SendBits,1);
			USART_SendData(USART1, Usart1SendBits);
    }
    else
    {
      //�����ֽڽ���			
      USART_ClearITPendingBit(USART1,USART_IT_TXE);
      USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
      USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    }
  }
	//�������
  if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
  {
    USART_ClearITPendingBit(USART1,USART_IT_TC);
    USART_ITConfig(USART1, USART_IT_TC, DISABLE);
  }
} 





void UART4_IRQHandler(void)//����2�жϷ������
{
	u8 Res;
	int value;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		Res =USART_ReceiveData(UART4);	//��ȡ���յ�������
		
		Usart4ReadBuff[Usart4ReadCnt] = Res;	//���յ����ݴ�������
		Usart4ReadCnt++;
		if(Usart4ReadCnt > Usart4ReadBuffLen -10)//��ֹ�������
		{
			Usart4ReadCnt = 0;
		}
	} 
	else if(USART_GetITStatus(UART4,USART_IT_IDLE) == SET)//�����ж�
	{
		UART4->DR; //���USART_IT_IDLE��־
		//�����ݴ��뻺��
	  BufferManageWrite(&buff_manage_struct_usart4,Usart4ReadBuff,Usart4ReadCnt,&value);
		
		
		PutData(&rb_t_usart1,"\r\n<<����:\r\n",11);
		PutData(&rb_t_usart1,Usart4ReadBuff,Usart4ReadCnt);
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		
		Usart4ReadCnt=0;
  }  
	
	if(USART_GetITStatus(UART4, USART_IT_TXE) != RESET)
  {
    if(Usart4SendDataCnt>0)//�����������ݸ�������0
    {
			USART_SendData(UART4, *Usart4SendData);
			Usart4SendData++;
		  Usart4SendDataCnt--;
    }
    else
    {
      //�����ֽڽ���			
			mymqtt.timer_out_send = 0;//�������ݽ���,���Է�����һ������
			
      USART_ClearITPendingBit(UART4,USART_IT_TXE);
      USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
      USART_ITConfig(UART4, USART_IT_TC, ENABLE);
    }
  }
	//�������
  if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
  {
    USART_ClearITPendingBit(UART4,USART_IT_TC);
    USART_ITConfig(UART4, USART_IT_TC, DISABLE);
  }
} 





/**
* @brief  ʹ��microLib�ķ���,ʹ��printf
* @warningg ��ѡ microLib
* @param  None
* @param  None
* @param  None
* @retval None
* @example 
**/

int fputc(int ch, FILE *f)
{
	static char flage=0;
	uint8_t data = ch;
	if(!flage)
	{
	  flage=1;
		USART_ClearITPendingBit(USART1,USART_IT_TC);//���printf��ʧ��һ���ֽڵ�����
	}
	PutData(&rb_t_usart1,&data,1);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
  return ch;
}

