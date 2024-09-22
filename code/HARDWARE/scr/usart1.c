/*-------------------------------------------------*/
/*            ModuleStar STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*             ʵ�ִ���1���ܵ�Դ�ļ�               */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "usart1.h"     //������Ҫ��ͷ�ļ�

#if  USART1_RX_ENABLE                   //���ʹ�ܽ��չ���
char Usart1_RxCompleted = 0;            //����һ������ 0����ʾ����δ��� 1����ʾ������� 
unsigned int Usart1_RxCounter = 0;      //����һ����������¼����1�ܹ������˶����ֽڵ�����
uint16_t data_receiving_counter=0;
char Usart1_RxBuff[USART1_RXBUFF_SIZE]; //����һ�����飬���ڱ��洮��1���յ�������   	
uint16_t DataBuff[2400];				//����һ�����飬���ڱ��洮��1���յ������ݣ�ÿ64���ֽ�Ϊһ�����ݣ���72��

uint8_t recv_data[2]={0,0};
uint8_t data_cut_flag=0;
uint8_t data_read_flag=0;

#endif

/*-------------------------------------------------*/
/*����������ʼ������1���͹���                      */
/*��  ����bound��������                            */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void Usart1_Init(unsigned int bound)
{  	 	
    GPIO_InitTypeDef GPIO_InitStructure;     //����һ������GPIO���ܵı���
	USART_InitTypeDef USART_InitStructure;   //����һ�����ô��ڹ��ܵı���
#if USART1_RX_ENABLE                         //���ʹ�ܽ��չ���
	NVIC_InitTypeDef NVIC_InitStructure;     //���ʹ�ܽ��չ��ܣ�����һ�������жϵı���
#endif

#if USART1_RX_ENABLE                                 //���ʹ�ܽ��չ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3
#endif	
      
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);  //ʹ�ܴ���1ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   //ʹ��GPIOAʱ��
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;              //׼������PA9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //IO����50M
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	       //����������������ڴ���1�ķ���
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //����PA9
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;             //׼������PA10 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //�������룬���ڴ���1�Ľ���
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //����PA10
	
	USART_InitStructure.USART_BaudRate = bound;                                    //����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //8������λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                         //1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;                            //����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
#if USART1_RX_ENABLE               												   //���ʹ�ܽ���ģʽ
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	               //�շ�ģʽ
#else                                                                              //�����ʹ�ܽ���ģʽ
	USART_InitStructure.USART_Mode = USART_Mode_Tx ;	                           //ֻ��ģʽ
#endif        
    USART_Init(USART1, &USART_InitStructure);                                      //���ô���1	

#if USART1_RX_ENABLE  	         					        //���ʹ�ܽ���ģʽ
	USART_ClearFlag(USART1, USART_FLAG_RXNE);	            //������ձ�־λ
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          //���������ж�
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       //���ô���1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	                        //���ô���1�ж�
#endif  

	USART_Cmd(USART1, ENABLE);                              //ʹ�ܴ���1
}

/*-------------------------------------------------*/
/*������������1 printf����                         */
/*��  ����char* fmt,...  ��ʽ������ַ����Ͳ���    */
/*����ֵ����                                       */
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
		recv_data[1]=p_data[counter++];                         //��������
		data_cut_flag=!data_cut_flag;
		if(recv_data[1]==0xff && recv_data[0]==0xff)			//������յ�0xff,0xff,��ʾ���ݿ�ʼ����
		{
			data_cut_flag=1;			//�Ӵ��������ݽضϵ�
			data_read_flag=1;			//��ʼ��ȡ����
			// data_receiving_counter=0;	//��ս������ݼ�����
		}
		else if(recv_data[1]==0xf0 && recv_data[0]==0xf0 && data_read_flag)			//������յ�0xf0f0����ʾ���ݽ������
		{
			DataBuff[data_receiving_counter++]=0xffff;//������־λ
			DataBuff[data_receiving_counter]=0;  //��һ��0��Ϊ����λ
			data_read_flag=0;			//ֹͣ��ȡ����
			// data_receiving_counter=0;	//��0�������ݼ�����
			// u1_printf((char *)DataBuff);
			// Usart1_RxCounter++;  //���ռ�����+1
		}
		else if(data_read_flag && data_cut_flag)			//������ڶ�ȡ�����Ҹýض�����
		{
			DataBuff[data_receiving_counter]=*(uint16_t*)recv_data;  //�����յ������ݴ���DataBuff
			data_receiving_counter++;  //�������ݼ���������1
		}
	}
}
