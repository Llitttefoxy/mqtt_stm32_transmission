/*-------------------------------------------------*/
/*            ModuleStar STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*              实现LED功能的头文件                */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __LED_H
#define __LED_H

#define STA_OUT(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)x)  //设置PB9 的电平，可以点亮熄灭LED

#define STA_IN_STA   GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_9) //PB9 控制LED3,读取电平状态，可以判断LED是点亮还是熄灭

#define STA_ON       GPIO_ResetBits(GPIOB, GPIO_Pin_9)        //共阳极，拉低PB9
#define STA_OFF      GPIO_SetBits(GPIOB, GPIO_Pin_9)          //共阳极，拉高PB9

void LED_LocationStaInit(void);               //初始化	
void LED_LocationStaOn(void);              //点亮所有LED
void LED_LocationStaOff(void);             //熄灭所有LED

#endif
