#include "System.h"
#include "usart3.h"

u32 rx3_cnt=0;
u8 rx3buffer[17];

void usart3_init(u32 bound)
{
      //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	  USART_InitTypeDef USART_InitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;
	 
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//ʹ��UART3ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD,ENABLE);//RCC_APB2Periph_AFIO
//    GPIO_PinRemapConfig(GPIO_FullRemap_USART3,ENABLE);
 	  USART_DeInit(USART3);  //��λ����3
	 //USART3_TX   PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB.10
   
    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ�� PB.11

   //Usart3 NVIC ����

      NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4 ;//��ռ���ȼ�3
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//�����ȼ�3
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	  NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	 USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	 USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	 USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	 USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(USART3, &USART_InitStructure); //��ʼ������
    
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 
}

void USART3_Send(u8 Data) //����һ���ֽڣ�
{
    USART_SendData(USART3,Data);
    while( USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET );
}

void USART3_Send_String(u8 *Data) //�����ַ�����
{
    while(*Data)
    USART3_Send(*Data++);
} 

unsigned char uart3_recByte(void) 
{ 	
	  uint16_t Data,t; 
	  while(!USART_GetFlagStatus(USART3,USART_FLAG_RXNE))
      {if(t++>1000) break;}
		Data=USART_ReceiveData(USART3);
		return (Data & 0xff); 	
}

void USART3_IRQHandler(void) //�жϴ�������
{
    u8 res;     
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //�ж��Ƿ����жϣ�
    {    
        res=USART_ReceiveData(USART3); //�������ݣ�     
        if(rx3_cnt<17)  
        {
            rx3buffer[rx3_cnt]=res; 
            rx3_cnt++;
        }
        if(rx3_cnt==17) rx3_cnt=0;         
    }  
    if(USART_GetFlagStatus(USART3,USART_FLAG_ORE)==SET)
    {
        USART_ReceiveData(USART3);
        USART_ClearFlag(USART3,USART_FLAG_ORE);        
    }
    USART_ClearITPendingBit(USART3, USART_IT_RXNE); //�����־λ��
} 


