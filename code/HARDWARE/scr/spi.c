/*-------------------------------------------------*/
/*            ModuleStar STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*              实现spi2功能的源文件               */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "delay.h"      //包含需要的头文件

/*-------------------------------------------------*/
/*函数名：重新映射JTAG口，复用成普通IO						   */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void JTAGRemapGPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);    
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);//PB3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);//PA15
}

/*-------------------------------------------------*/
/*函数名：初始化SPI接口                            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;                      //定义一个设置GPIO的变量
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,ENABLE);   //使能GPIOB/A端口时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);    //使能SPI2时钟 
	
	GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;//准备设置PB13 PB14 PB15
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;                      //速率50Mhz
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;                        //复用推免输出模式
	GPIO_Init(GPIOB, &GPIO_InitStruct);                                 //设置PB13 PB14 PB15
	/*定义SCS引脚*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;        //准备设置PB12
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; //速率50Mhz
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  //推免输出模式
	GPIO_Init(GPIOB, &GPIO_InitStruct);            //设置PB12 
	GPIO_SetBits(GPIOB,GPIO_Pin_12);               //PB12置高电平

	/*定义RESET引脚*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;					 /*选择要控制的GPIO引脚*/		 
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		     /*设置引脚速率为50MHz */		
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;		     /*设置引脚模式为通用推挽输出*/	
	GPIO_Init(GPIOB, &GPIO_InitStruct);		 /*调用库函数，初始化GPIO*/
	GPIO_SetBits(GPIOB, GPIO_Pin_10);		

	/*定义INT引脚*/	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;						 /*选择要控制的GPIO引脚*/		 
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		     /*设置引脚速率为50MHz*/		
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;				 /*设置引脚模式为通用推挽模拟上拉输入*/		
	GPIO_Init(GPIOA, &GPIO_InitStruct);			 /*调用库函数，初始化GPIO*/

	JTAGRemapGPIOInit();
	// 加上会提示网线未连接TODO
	// GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	// Delay_Ms(1);  
	// GPIO_SetBits(GPIOA, GPIO_Pin_11);
	// Delay_Ms(10);
}

/*-------------------------------------------------*/
/*函数名：初始化SPI功能                            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStruct;
	SPI_GPIO_Configuration();	
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2,&SPI_InitStruct);	
	SPI_SSOutputCmd(SPI2, ENABLE);
	SPI_Cmd(SPI2, ENABLE);
}
/*-------------------------------------------------*/
/*函数名：写1字节数据到SPI总线                     */
/*参  数：TxData:写到总线的数据                    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_WriteByte(unsigned char TxData)
{				 
	while((SPI2->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空		  
	SPI2->DR=TxData;	 	  				//发送一个byte 
	while((SPI2->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	SPI2->DR;		
}

/*-------------------------------------------------*/
/*函数名：从SPI总线读取1字节数据                   */
/*参  数：无                                       */
/*返回值：读到的数据                               */
/*-------------------------------------------------*/
unsigned char SPI_ReadByte(void)
{			 
	while((SPI2->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空			  
	SPI2->DR=0xFF;	 	  					//发送一个空数据产生输入数据的时钟 
	while((SPI2->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	return SPI2->DR;  						    
}
/*-------------------------------------------------*/
/*函数名：进入临界区                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);    //禁止全局中断
}
/*-------------------------------------------------*/
/*函数名：退出临界区                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);   //开全局中断
}
/*-------------------------------------------------*/
/*函数名： 片选信号输出低电平                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CS_Select(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

/*-------------------------------------------------*/
/*函数名： 片选信号输出高电平                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CS_Deselect(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}
