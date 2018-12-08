/*
 * SonicRadar.c
 *
 *  Created on: 2018-12-7
 *      Author: zhtro
 */
#include <xdc/std.h>
#include "uartns550.h"
#include "xil_types.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include "xdc/runtime/System.h"
#include "Message/Message.h"
#include <ti/sysbios/knl/Task.h>

#define SONIC_UARTNUM 1

/****************************************************************************/
/*                                                                          */
/*         变量                                                        */
/*                                                                          */
/****************************************************************************/
static Semaphore_Handle sem_sonicRadar;
static uint16_t m_distance;
/****************************************************************************/
/*                                                                          */
/*              函数声明                                                        */
/*                                                                          */
/****************************************************************************/

void SonicRadarUartIntrHandler(void *CallBackRef, u32 Event, unsigned int EventData);

/****************************************************************************/
/*                                                                          */
/*              函数定义                                                        */
/*                                                                          */
/****************************************************************************/
Void taskSonicRadar(UArg a0, UArg a1)
{
	Semaphore_Params semParams;
	uint8_t Sonic_query[3] = {0xe8,0x02,0xb4};
	UART550_BUFFER * Buf;
	p_msg_t pmsg;

	//初始化串口
	UartNs550Init(SONIC_UARTNUM,SonicRadarUartIntrHandler);


	//创建信号量
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	sem_sonicRadar = Semaphore_create(0, &semParams, NULL);

	while(1)
	{
		Task_sleep(1000);

		UartNs550Send(SONIC_UARTNUM, Sonic_query, 3);

		Semaphore_pend(sem_sonicRadar,BIOS_WAIT_FOREVER);

		Buf = UartNs550PopBuffer(SONIC_UARTNUM);

		if(Buf->Length != 2)
		{
			System_printf("Sonic Radar error: recved %d bytes\n", Buf->Length);
			BIOS_exit(0);
		}

		m_distance = Buf->Buffer[0] * 256 + Buf->Buffer[1];

		pmsg = Message_getEmpty();
		pmsg->type = sonicradar;
		memcpy( pmsg->data , &m_distance ,sizeof(m_distance));
		pmsg->dataLen = sizeof(m_distance);

		Message_post(pmsg);

	}


}


uint16_t SonicGetDistance()
{
	return m_distance;
}

void SonicRadarUartIntrHandler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u8 Errors;
	u16 DeviceNum = *((u16 *)CallBackRef);
    u8 *NextBuffer;

	/*
	 * All of the data has been sent.
	 */
	if (Event == XUN_EVENT_SENT_DATA) {
//		TotalSentCount = EventData;

	}

	/*
	 * All of the data has been received.
	 */
	if (Event == XUN_EVENT_RECV_DATA) {
//		TotalReceivedCount = EventData;
        NextBuffer = UartNs550PushBuffer(DeviceNum,EventData);
        UartNs550Recv(DeviceNum, NextBuffer, BUFFER_MAX_SIZE);
        Semaphore_post(sem_sonicRadar);
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_TIMEOUT) {
        NextBuffer = UartNs550PushBuffer(DeviceNum,EventData);
        UartNs550Recv(DeviceNum, NextBuffer, BUFFER_MAX_SIZE);
//		TotalReceivedCount = EventData;
		 Semaphore_post(sem_sonicRadar);
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred.
	 */
	if (Event == XUN_EVENT_RECV_ERROR) {
//		TotalReceivedCount = EventData;
//		TotalErrorCount++;
		Errors = UartNs550GetLastErrors(DeviceNum);
	}
}
