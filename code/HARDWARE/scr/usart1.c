/*-------------------------------------------------*/
/*            ModuleStar STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*             实现串口1功能的源文件               */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "usart1.h"     //包含需要的头文件

#if  USART1_RX_ENABLE                   //如果使能接收功能
char Usart1_RxCompleted = 0;            //定义一个变量 0：表示接收未完成 1：表示接收完成 
unsigned int Usart1_RxCounter = 0;      //定义一个变量，记录串口1总共接收了多少字节的数据
uint16_t data_receiving_counter=0;
char Usart1_RxBuff[USART1_RXBUFF_SIZE]; //定义一个数组，用于保存串口1接收到的数据   	
uint16_t DataBuff[2400];				//定义一个数组，用于保存串口1接收到的数据，每64个字节为一组数据，共72组

uint8_t recv_data[2]={0,0};
uint8_t data_cut_flag=0;
uint8_t data_read_flag=0;

#endif

/*-------------------------------------------------*/
/*函数名：初始化串口1发送功能                      */
/*参  数：bound：波特率                            */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Usart1_Init(unsigned int bound)
{  	 	
    GPIO_InitTypeDef GPIO_InitStructure;     //定义一个设置GPIO功能的变量
	USART_InitTypeDef USART_InitStructure;   //定义一个设置串口功能的变量
#if USART1_RX_ENABLE                         //如果使能接收功能
	NVIC_InitTypeDef NVIC_InitStructure;     //如果使能接收功能，定义一个设置中断的变量
#endif

#if USART1_RX_ENABLE                                 //如果使能接收功能
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //设置中断向量分组：第2组 抢先优先级：0 1 2 3 子优先级：0 1 2 3
#endif	
      
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);  //使能串口1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   //使能GPIOA时钟
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;              //准备设置PA9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //IO速率50M
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	       //复用推挽输出，用于串口1的发送
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //设置PA9
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;             //准备设置PA10 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入，用于串口1的接收
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //设置PA10
	
	USART_InitStructure.USART_BaudRate = bound;                                    //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //8个数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                         //1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;                            //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
#if USART1_RX_ENABLE               												   //如果使能接收模式
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	               //收发模式
#else                                                                              //如果不使能接收模式
	USART_InitStructure.USART_Mode = USART_Mode_Tx ;	                           //只发模式
#endif        
    USART_Init(USART1, &USART_InitStructure);                                      //设置串口1	

#if USART1_RX_ENABLE  	         					        //如果使能接收模式
	USART_ClearFlag(USART1, USART_FLAG_RXNE);	            //清除接收标志位
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          //开启接收中断
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       //设置串口1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//中断通道使能
	NVIC_Init(&NVIC_InitStructure);	                        //设置串口1中断
#endif  

	USART_Cmd(USART1, ENABLE);                              //使能串口1
}

/*-------------------------------------------------*/
/*函数名：串口1 printf函数                         */
/*参  数：char* fmt,...  格式化输出字符串和参数    */
/*返回值：无                                       */
/*-------------------------------------------------*/

__align(8) char Usart1_TxBuff[USART1_TXBUFF_SIZE];  

void u1_printf(char* fmt,...) 
{  
	unsigned int i,length;
	
	va_list ap;
	va_start(ap,fmt);
	vsprintf(Usart1_TxBuff,fmt,ap);
	va_end(ap);	
	
	length=strlen((const char*)Usart1_TxBuff);		
	while((USART1->SR&0X40)==0);
	for(i = 0;i < length;i ++)
	{			
		USART1->DR = Usart1_TxBuff[i];
		while((USART1->SR&0X40)==0);	
	}	
}

void DataProcess(void)
{
	uint16_t counter=0, Total_LEN;
	uint8_t* p_data=(uint8_t*)DataBuff;
	Total_LEN=4800-DMA_GetCurrDataCounter(DMA1_Channel5);
	while(1)
	{
		if(counter==Total_LEN)
		{
			break;
		}
		
		recv_data[0]=recv_data[1];
		recv_data[1]=p_data[counter++];                         //接收数据
		data_cut_flag=!data_cut_flag;
		if(recv_data[1]==0xff && recv_data[0]==0xff)			//如果接收到0xff,0xff,表示数据开始接收
		{
			data_cut_flag=1;			//从此认作数据截断点
			data_read_flag=1;			//开始读取数据
			// data_receiving_counter=0;	//清空接收数据计数器
		}
		else if(recv_data[1]==0xf0 && recv_data[0]==0xf0 && data_read_flag)			//如果接收到0xf0f0，表示数据接收完毕
		{
			DataBuff[data_receiving_counter++]=0xffff;//结束标志位
			DataBuff[data_receiving_counter]=0;  //存一个0作为结束位
			data_read_flag=0;			//停止读取数据
			// data_receiving_counter=0;	//清0接收数据计数器
			// u1_printf((char *)DataBuff);
			// Usart1_RxCounter++;  //接收计数器+1
		}
		else if(data_read_flag && data_cut_flag)			//如果正在读取数据且该截断数据
		{
			DataBuff[data_receiving_counter]=*(uint16_t*)recv_data;  //将接收到的数据存入DataBuff
			data_receiving_counter++;  //接收数据计数器自增1
		}
	}
}
