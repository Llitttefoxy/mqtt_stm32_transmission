


#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "dma.h"     //������Ҫ��ͷ�ļ�
#include "usart1.h"

void My_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //ʹ��DMA1ʱ��
    
    DMA_DeInit(DMA1_Channel5); //��λDMAͨ��5
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //DMA�����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DataBuff; //DMA�ڴ��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //���ݴ����赽�ڴ�
    DMA_InitStructure.DMA_BufferSize = 4800; //���ݳ���
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //�ڴ��ַ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //�������ݳ���Ϊ�ֽ�
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //�ڴ����ݳ���Ϊ�ֽ�
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //��ͨģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //�����ȼ�
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //��ʹ���ڴ浽�ڴ�ģʽ
    
    DMA_Init(DMA1_Channel5, &DMA_InitStructure); //��ʼ��DMAͨ��1
    
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); //ʹ��USART1��DMA��������

    // DMA_Cmd(DMA1_Channel5, ENABLE); //ʹ��DMAͨ��
}

void MYDMA_Enable(void)
{
    DMA_Cmd(DMA1_Channel5, DISABLE ); //�ر� USART1 TX DMA1 ��ָʾ��ͨ��
    DMA_SetCurrDataCounter(DMA1_Channel5,4800);//���� DMA ����Ĵ�С
    DMA_Cmd(DMA1_Channel5, ENABLE); //ʹ�� USART1 TX DMA1 ��ָʾ��ͨ��
}
