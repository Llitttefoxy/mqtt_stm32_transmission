/*-------------------------------------------------*/
/*            ModuleStar STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*            实现扫描按键功能的源文件             */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "delay.h"      //包含需要的头文件
#include "key.h"      //包含需要的头文件

/*-------------------------------------------------*/
/*函数名：初始化KEY按键函数                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void KEY_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;                       //定义一个设置IO的结构体
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);    //使能PA端口时钟
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE);    //使能PA端口时钟

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
} 

/*-------------------------------------------------*/
/*函数名：按键扫描函数                             */
/*参  数：无                                       */
/*返回值：有无按键按下，有的话是哪个按键按下       */
/*-------------------------------------------------*/
unsigned char KEY_Scan(void)
{
	static unsigned char key1_In=0;
	static unsigned char key2_In=0;
	unsigned char keyTmp1;
	unsigned char keyTmp2;

	if(KEY1_IN_STA==0){
		Delay_Ms(100);
		if(KEY1_IN_STA==0){	
			keyTmp1 = 1;
			if (key1_In != keyTmp1) {
				key1_In = keyTmp1;
				return key1_In;
			}
			return 0xff;
		}
	} else {
		key1_In=0;
	}

	if(KEY2_IN_STA==0){
		Delay_Ms(100);
		if(KEY2_IN_STA==0){	
			keyTmp2 = 2;
			if (key2_In != keyTmp2) {
				key2_In = keyTmp2;
				return key2_In;
			}
			return 0xff;
		}
	} else {
		key2_In=0;
	}
	return 0xff;
}

/*-------------------------------------------------*/
/*函数名：按键外部中断初始化程序.                    */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void KEY_Exti_Init(void)
{
	// NVIC_InitTypeDef   NVIC_InitStructure; //定义一个设置IO的结构体
	// EXTI_InitTypeDef   EXTI_InitStructure; //定义一个设置中断的结构体 

  	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	          //使能GPIO复用功能时钟
  	// GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);    //配置KEY1-PA0中断线
	// GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);    //配置KEY2-PA1中断线

  	// EXTI_InitStructure.EXTI_Line = EXTI_Line0;	                  //准备设置外部0中断线
  	// EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	          //外部中断
  	// EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;        //上升沿触发中断
  	// EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //使能
  	// EXTI_Init(&EXTI_InitStructure);	 	                          //设置外部中断0线
	
  	// EXTI_InitStructure.EXTI_Line = EXTI_Line1;	                  //准备设置KEY2-PA1外部1中断线
  	// EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	          //外部中断
  	// EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;       //下降沿触发中断
  	// EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //使能
  	// EXTI_Init(&EXTI_InitStructure);	 	                          //设置外部中断1线

	// NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			  //准备设置外部中断0
  	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     //抢占优先级3 
  	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		      //子优先级0
  	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //使能外部中断0通道
  	// NVIC_Init(&NVIC_InitStructure);                               //设置外部中断0

    // NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			  //准备设置外部中断1
  	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     //抢占优先级3
  	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		      //子优先级1
  	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				  //使能外部中断1通道
  	// NVIC_Init(&NVIC_InitStructure);                               //设置外部中断1
}




















