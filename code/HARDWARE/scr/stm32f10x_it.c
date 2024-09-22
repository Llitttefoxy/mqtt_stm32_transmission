/*-------------------------------------------------*/
/*            ModuleStar STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*          实现各种中断服务函数的源文件           */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"     //包含需要的头文件
#include "stm32f10x_it.h"  //包含需要的头文件
#include "main.h"          //包含需要的头文件
#include "delay.h"         //包含需要的头文件
#include "usart1.h"        //包含需要的头文件
#include "timer1.h"        //包含需要的头文件
#include "timer3.h"        //包含需要的头文件
#include "led.h"           //包含需要的头文件
#include "mqtt.h"          //包含需要的头文件
#include "dma.h"

// uint8_t recv_data[2]={0,0};
// uint8_t data_cut_flag=0;
// uint8_t data_read_flag=0;

/*-------------------------------------------------*/
/*函数名：USART1中断服务函数                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void USART1_IRQHandler(void)
{
	// if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){  //如果USART_IT_RXNE置位，表示接收数据中断，进入if
	// 	USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //清除USART1接收数据中断标志
	// 	recv_data[0]=recv_data[1];
	// 	recv_data[1]=USART_ReceiveData(USART1);                         //接收数据
	// 	data_cut_flag=!data_cut_flag;
	// 	if(recv_data[1]==0xff && recv_data[0]==0xff)			//如果接收到0xff,0xff,表示数据开始接收
	// 	{
	// 		data_cut_flag=1;			//从此认作数据截断点
	// 		data_read_flag=1;			//开始读取数据
	// 		// data_receiving_counter=0;	//清空接收数据计数器
	// 	}
	// 	else if(recv_data[1]==0xf0 && recv_data[0]==0xf0 && data_read_flag)			//如果接收到0xf0f0，表示数据接收完毕
	// 	{
	// 		DataBuff[data_receiving_counter++]=0xffff;//结束标志位
	// 		DataBuff[data_receiving_counter]=0;  //存一个0作为结束位
	// 		data_read_flag=0;			//停止读取数据
	// 		// data_receiving_counter=0;	//清0接收数据计数器
	// 		u1_printf((char *)DataBuff);
	// 		// Usart1_RxCounter++;  //接收计数器+1
	// 	}
	// 	else if(data_read_flag && data_cut_flag)			//如果正在读取数据且该截断数据
	// 	{
	// 		DataBuff[data_receiving_counter]=*(uint16_t*)recv_data;  //将接收到的数据存入DataBuff
	// 		data_receiving_counter++;  //接收数据计数器自增1
	// 	}
	// }
}
/*
void USART1_IRQHandler(void)
{
	uint8_t recv_data;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){  //如果USART_IT_RXNE置位，表示接收数据中断，进入if
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //清除USART1接收数据中断标志
		recv_data=USART_ReceiveData(USART1);                         //接收数据
		if(recv_data=='\r' && data_receiving_counter==2)
		{
			// Usart1_RxCounter++;  //接收计数器+1
			data_receiving_counter=0;  //清空接收数据计数器
			MQTT_UploadData(DataBuff[0]);

		}
		else
		{
			DataBuff[Usart1_RxCounter][data_receiving_counter]=recv_data;  //将接收到的数据存入DataBuff
			data_receiving_counter++;  //接收数据计数器自增1
		}
		USART_SendData(USART1, recv_data);                         //回发数据
		// u1_printf("data received:%c\r\n",recv_data);  //回发数据
	}
}
*/

/*-------------------------------------------------*/
/*函数名：定时器3中断服务函数                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM3_IRQHandler(void)
{
	// uint8_t i=0;
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){   //如果TIM_IT_Update置位，表示TIM3溢出中断，进入if
		DMA_Cmd(DMA1_Channel5, DISABLE );	

		u1_printf("enteringit\r\n");
		// USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);          //关闭接收中断,漏几个也没关系
		MQTT_PingREQ(); 					             //添加Ping报文到发送缓冲区           		             		 //Ping_flag自增1，表示又发送了一次ping，期待服务器的回复
		// MQTT_UploadString('k');

		DataProcess();  //处理接收到的数据
		u1_printf("endDataProcess\r\n");
		// u1_printf((char *)DataBuff);
		if(data_receiving_counter)
		{
			MQTT_PublishQs0(P_TOPIC_NAME, (char *)DataBuff, 2*data_receiving_counter);
			u1_printf("total:%d\r\n",data_receiving_counter);
			// Usart1_RxCounter=0;  //清空接收计数器
			data_receiving_counter=0;//清空接收数据计数器
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);      //清除TIM3溢出中断标志 	
		// USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);          //开启接收中断
		u1_printf("exitingit\r\n");
		MYDMA_Enable();
	}
}
/*-------------------------------------------------*/
/*函数名：不可屏蔽中断处理函数                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void NMI_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：硬件出错后进入的中断处理函数             */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void HardFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：内存管理中断处理函数                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MemManage_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：预取指失败，存储器访问失败中断处理函数   */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void BusFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：未定义的指令或非法状态处理函数           */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void UsageFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：软中断，SWI 指令调用的处理函数           */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SVC_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*函数名：调试监控器处理函数                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void DebugMon_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*函数名：可挂起的系统服务处理函数                 */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void PendSV_Handler(void)
{
}

/*-------------------------------------------------*/
/*函数名：SysTic系统嘀嗒定时器处理函数             */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SysTick_Handler(void)
{
 
}
