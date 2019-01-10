/*
 * HWI_4_Isr.c
 *
 *  HWI 4 中断处理函数
 *  处理GPIO bank 0中断
 *  FPGA的UART 和 CAN中断都在这里处理
 *
 *  Created on: 2018-12-3
 *      Author: zhtro
 */

#include "interrupt.h"
#include "soc_C6748.h"
#include "mpu9250_iic.h"

extern void IICInterruptHandler(IICObj_t *insPtr);

extern IICObj_t mpu9250IICInst;

void HWI_6_Isr(void)
{
     IICInterruptHandler(&mpu9250IICInst);
}
