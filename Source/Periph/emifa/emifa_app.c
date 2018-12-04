/*
 * emifa_app.c
 *
 *  Created on: 2018��11��27��
 *      Author: hk
 */

#include <emifa.h>
#include "stdint.h"
#include "hw_types.h"               // ������
#include "hw_syscfg0_C6748.h"       // ϵͳ����ģ��Ĵ���
#include "soc_C6748.h"              // DSP C6748 ����Ĵ���
#include "psc.h"                    // ��Դ��˯�߿��ƺ꼰�豸����㺯������
#include "gpio.h"                   // ͨ����������ں꼰�豸����㺯������
#include "rtc.h"                    // ʵʱʱ�Ӻ꼰�豸����㺯������
#include "interrupt.h"             // DSP C6748 �ж����Ӧ�ó���ӿں���������ϵͳ�¼��Ŷ���
#include "emifa/EMIFAPinmuxSetup.h"
#include "uartStdio.h"             // ���ڱ�׼��������ն˺�������


/****************************************************************************/
/*                                                                          */
/*              ��ʼ��EMIFA                                           */
/*                                                                          */
/****************************************************************************/

static void PSCInit(void)
{

	PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_EMIFA, PSC_POWERDOMAIN_ALWAYS_ON,
					 PSC_MDCTL_NEXT_ENABLE);
}

static void EMIFASetup(void)
{

	EMIFAPinMuxSetup();


	EMIFAAsyncDevDataBusWidthSelect(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
									EMIFA_DATA_BUSWITTH_16BIT);

	/*异步Normal模式*/
	EMIFAAsyncDevOpModeSelect(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
							   EMIFA_ASYNC_INTERFACE_NORMAL_MODE);

	/* Disable WAIT信号 */
	EMIFAExtendedWaitConfig(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
							 EMIFA_EXTENDED_WAIT_DISABLE);

	/* 时序参数W_SETUP/R_SETUP   W_STROBE/R_STROBE    W_HOLD/R_HOLD	TA */
	EMIFAWaitTimingConfig(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
						   EMIFA_ASYNC_WAITTIME_CONFIG(0, 3, 1, 1, 8, 2, 0 ));
}

uint8_t EMIFAReadUart(uintptr_t *Addr,uint8_t offset)
{
    return *((volatile uint16_t *)Addr+offset);
}

void EMIFAWriteUart(uintptr_t Addr,uint8_t offset,uint8_t Value)
{
    volatile uint16_t *LocalAddr = (volatile uint16_t *)Addr+offset;
    *LocalAddr = Value;
}

void EMIFA_init()
{
	PSCInit();
	EMIFASetup();
}


