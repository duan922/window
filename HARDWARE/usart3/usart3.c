#include "System.h"
#include "usart3.h"

u32 rx3_cnt=0;
u8 rx3buffer[17];

void usart3_init(u32 bound)
{
      //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	  USART_InitTypeDef USART_InitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;
	 
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能UART3时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD,ENABLE);//RCC_APB2Periph_AFIO
//    GPIO_PinRemapConfig(GPIO_FullRemap_USART3,ENABLE);
 	  USART_DeInit(USART3);  //复位串口3
	 //USART3_TX   PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB.10
   
    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化 PB.11

   //Usart3 NVIC 配置

      NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4 ;//抢占优先级3
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//子优先级3
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	  NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	 USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	 USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	 USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	 USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART3, &USART_InitStructure); //初始化串口
    
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART3, ENABLE);                    //使能串口 
}

void USART3_Send(u8 Data) //发送一个字节；
{
    USART_SendData(USART3,Data);
    while( USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET );
}

void USART3_Send_String(u8 *Data) //发送字符串；
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

void USART3_IRQHandler(void) //中断处理函数；
{
    u8 res;     
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //判断是否发生中断；
    {    
        res=USART_ReceiveData(USART3); //接收数据；     
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
    USART_ClearITPendingBit(USART3, USART_IT_RXNE); //清除标志位；
} 


