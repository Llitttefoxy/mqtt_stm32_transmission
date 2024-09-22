/*-------------------------------------------------*/
/*            ModuleStar STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*          ʵ�ָ����жϷ�������Դ�ļ�           */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"     //������Ҫ��ͷ�ļ�
#include "stm32f10x_it.h"  //������Ҫ��ͷ�ļ�
#include "main.h"          //������Ҫ��ͷ�ļ�
#include "delay.h"         //������Ҫ��ͷ�ļ�
#include "usart1.h"        //������Ҫ��ͷ�ļ�
#include "timer1.h"        //������Ҫ��ͷ�ļ�
#include "timer3.h"        //������Ҫ��ͷ�ļ�
#include "led.h"           //������Ҫ��ͷ�ļ�
#include "mqtt.h"          //������Ҫ��ͷ�ļ�
#include "dma.h"

// uint8_t recv_data[2]={0,0};
// uint8_t data_cut_flag=0;
// uint8_t data_read_flag=0;

/*-------------------------------------------------*/
/*��������USART1�жϷ�����                        */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void USART1_IRQHandler(void)
{
	// if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){  //���USART_IT_RXNE��λ����ʾ���������жϣ�����if
	// 	USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //���USART1���������жϱ�־
	// 	recv_data[0]=recv_data[1];
	// 	recv_data[1]=USART_ReceiveData(USART1);                         //��������
	// 	data_cut_flag=!data_cut_flag;
	// 	if(recv_data[1]==0xff && recv_data[0]==0xff)			//������յ�0xff,0xff,��ʾ���ݿ�ʼ����
	// 	{
	// 		data_cut_flag=1;			//�Ӵ��������ݽضϵ�
	// 		data_read_flag=1;			//��ʼ��ȡ����
	// 		// data_receiving_counter=0;	//��ս������ݼ�����
	// 	}
	// 	else if(recv_data[1]==0xf0 && recv_data[0]==0xf0 && data_read_flag)			//������յ�0xf0f0����ʾ���ݽ������
	// 	{
	// 		DataBuff[data_receiving_counter++]=0xffff;//������־λ
	// 		DataBuff[data_receiving_counter]=0;  //��һ��0��Ϊ����λ
	// 		data_read_flag=0;			//ֹͣ��ȡ����
	// 		// data_receiving_counter=0;	//��0�������ݼ�����
	// 		u1_printf((char *)DataBuff);
	// 		// Usart1_RxCounter++;  //���ռ�����+1
	// 	}
	// 	else if(data_read_flag && data_cut_flag)			//������ڶ�ȡ�����Ҹýض�����
	// 	{
	// 		DataBuff[data_receiving_counter]=*(uint16_t*)recv_data;  //�����յ������ݴ���DataBuff
	// 		data_receiving_counter++;  //�������ݼ���������1
	// 	}
	// }
}
/*
void USART1_IRQHandler(void)
{
	uint8_t recv_data;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){  //���USART_IT_RXNE��λ����ʾ���������жϣ�����if
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //���USART1���������жϱ�־
		recv_data=USART_ReceiveData(USART1);                         //��������
		if(recv_data=='\r' && data_receiving_counter==2)
		{
			// Usart1_RxCounter++;  //���ռ�����+1
			data_receiving_counter=0;  //��ս������ݼ�����
			MQTT_UploadData(DataBuff[0]);

		}
		else
		{
			DataBuff[Usart1_RxCounter][data_receiving_counter]=recv_data;  //�����յ������ݴ���DataBuff
			data_receiving_counter++;  //�������ݼ���������1
		}
		USART_SendData(USART1, recv_data);                         //�ط�����
		// u1_printf("data received:%c\r\n",recv_data);  //�ط�����
	}
}
*/

/*-------------------------------------------------*/
/*����������ʱ��3�жϷ�����                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM3_IRQHandler(void)
{
	// uint8_t i=0;
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){   //���TIM_IT_Update��λ����ʾTIM3����жϣ�����if
		DMA_Cmd(DMA1_Channel5, DISABLE );	

		u1_printf("enteringit\r\n");
		// USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);          //�رս����ж�,©����Ҳû��ϵ
		MQTT_PingREQ(); 					             //���Ping���ĵ����ͻ�����           		             		 //Ping_flag����1����ʾ�ַ�����һ��ping���ڴ��������Ļظ�
		// MQTT_UploadString('k');

		DataProcess();  //������յ�������
		u1_printf("endDataProcess\r\n");
		// u1_printf((char *)DataBuff);
		if(data_receiving_counter)
		{
			MQTT_PublishQs0(P_TOPIC_NAME, (char *)DataBuff, 2*data_receiving_counter);
			u1_printf("total:%d\r\n",data_receiving_counter);
			// Usart1_RxCounter=0;  //��ս��ռ�����
			data_receiving_counter=0;//��ս������ݼ�����
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);      //���TIM3����жϱ�־ 	
		// USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          //���������ж�
		u1_printf("exitingit\r\n");
		MYDMA_Enable();
	}
}
/*-------------------------------------------------*/
/*�����������������жϴ�����                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void NMI_Handler(void)
{

}

/*-------------------------------------------------*/
/*��������Ӳ������������жϴ�����             */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void HardFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*���������ڴ�����жϴ�����                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void MemManage_Handler(void)
{

}

/*-------------------------------------------------*/
/*��������Ԥȡָʧ�ܣ��洢������ʧ���жϴ�����   */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void BusFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*��������δ�����ָ���Ƿ�״̬������           */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void UsageFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*�����������жϣ�SWI ָ����õĴ�����           */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SVC_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*�����������Լ����������                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void DebugMon_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*���������ɹ����ϵͳ��������                 */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void PendSV_Handler(void)
{
}

/*-------------------------------------------------*/
/*��������SysTicϵͳ��શ�ʱ��������             */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SysTick_Handler(void)
{
 
}
