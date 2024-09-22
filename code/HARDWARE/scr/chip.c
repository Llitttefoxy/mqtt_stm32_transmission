


#include "stm32f10x.h"  //包含需要的头文件
#include "chip.h"

void GetChipID(uint32_t *id)
{
    id[2] = *(volatile uint32_t*)(0X1FFFF7E8);  // 低字节
    id[1] = *(volatile uint32_t *)(0X1FFFF7EC); // 
    id[0] = *(volatile uint32_t *)(0X1FFFF7F0); // 高字节
}
