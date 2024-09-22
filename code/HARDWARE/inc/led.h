/*-------------------------------------------------*/
/*            ModuleStar STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*              ʵ��LED���ܵ�ͷ�ļ�                */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __LED_H
#define __LED_H

#define STA_OUT(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)x)  //����PB9 �ĵ�ƽ�����Ե���Ϩ��LED

#define STA_IN_STA   GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_9) //PB9 ����LED3,��ȡ��ƽ״̬�������ж�LED�ǵ�������Ϩ��

#define STA_ON       GPIO_ResetBits(GPIOB, GPIO_Pin_9)        //������������PB9
#define STA_OFF      GPIO_SetBits(GPIOB, GPIO_Pin_9)          //������������PB9

void LED_LocationStaInit(void);               //��ʼ��	
void LED_LocationStaOn(void);              //��������LED
void LED_LocationStaOff(void);             //Ϩ������LED

#endif
