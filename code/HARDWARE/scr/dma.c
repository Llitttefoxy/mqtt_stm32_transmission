


#include "stm32f10x.h"  //包含需要的头文件
#include "dma.h"     //包含需要的头文件
#include "usart1.h"

void My_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //使能DMA1时钟
    
    DMA_DeInit(DMA1_Channel5); //复位DMA通道5
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //DMA外设地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DataBuff; //DMA内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //数据从外设到内存
    DMA_InitStructure.DMA_BufferSize = 4800; //数据长度
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址不自增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址自增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据长度为字节
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //内存数据长度为字节
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //普通模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //高优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //不使用内存到内存模式
    
    DMA_Init(DMA1_Channel5, &DMA_InitStructure); //初始化DMA通道1
    
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); //使能USART1的DMA接收请求

    // DMA_Cmd(DMA1_Channel5, ENABLE); //使能DMA通道
}

void MYDMA_Enable(void)
{
    DMA_Cmd(DMA1_Channel5, DISABLE ); //关闭 USART1 TX DMA1 所指示的通道
    DMA_SetCurrDataCounter(DMA1_Channel5,4800);//设置 DMA 缓存的大小
    DMA_Cmd(DMA1_Channel5, ENABLE); //使能 USART1 TX DMA1 所指示的通道
}
