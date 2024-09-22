/*-------------------------------------------------*/
/*            ModuleStar STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*            实现扫描按键功能的头文件             */
/*                                                 */
/*-------------------------------------------------*/
#ifndef __KEY_H
#define __KEY_H	

#define KEY1_IN_STA  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)
#define KEY2_IN_STA  GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)

void KEY_Init(void);	   //按键初始化
unsigned char KEY_Scan(void);       //按键扫描函数	
void KEY_Exti_Init(void);  //按键外部中断初始化程序

#endif
