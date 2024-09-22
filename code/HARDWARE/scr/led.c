/*-------------------------------------------------*/
/*            ModuleStar STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*              ʵ��LED���ܵ�Դ�ļ�                */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "led.h"        //������Ҫ��ͷ�ļ�

/*-------------------------------------------------*/
/*����������ʼ��LED����                            */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LED_LocationStaInit(void)
{    	 
	GPIO_InitTypeDef GPIO_InitStructure;                     //����һ������GPIO�ı���
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);                                //ʹ��GPIOB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                //׼������B9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                                     //IO����50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   		                              //���������ʽ
	GPIO_Init(GPIOB, &GPIO_InitStructure);            		                              //����PB9
}

/*-------------------------------------------------*/
/*����������������LED                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LED_LocationStaOn(void)
{
	STA_ON;
}

/*-------------------------------------------------*/
/*��������Ϩ������LED                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LED_LocationStaOff(void)
{
	STA_OFF;
}



