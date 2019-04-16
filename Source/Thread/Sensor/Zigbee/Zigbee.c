/*
 * Zigbee.c
 *
 *  Created on: 2019-4-14
 *      Author: zhtro
 */

#include "Zigbee/Zigbee.h"
#include "uartns550.h"
#include <ti/sysbios/knl/Semaphore.h>
#include "stdint.h"
#include "string.h"
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/BIOS.h>
#include "Message/Message.h"


static Semaphore_Handle m_sem_zigbee_recv;
static Semaphore_Handle m_sem_zigbee_send;





static void UartZigbeeIntrHandler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u8 Errors;
	u16 UartDeviceNum = *((u16 *)CallBackRef);
    u8 *NextBuffer;
	/*
	 * All of the data has been sent.
	 */
	if (Event == XUN_EVENT_SENT_DATA) {
//		TotalSentCount = EventData;
		Semaphore_post(m_sem_zigbee_send);
	}

	/*
	 * All of the data has been received.
	 */
	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_DATA || Event == XUN_EVENT_RECV_TIMEOUT) {
		NextBuffer = UartNs550PushBuffer(UartDeviceNum,EventData);
		UartNs550Recv(UartDeviceNum, NextBuffer, BUFFER_MAX_SIZE);
		Semaphore_post(m_sem_zigbee_recv);
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred.
	 */
	if (Event == XUN_EVENT_RECV_ERROR) {
//		TotalReceivedCount = EventData;
//		TotalErrorCount++;
		Errors = UartNs550GetLastErrors(UartDeviceNum);
	}
}

void ZigbeeInit()
{
	Semaphore_Params semParams;


	/*初始化FPGA串口*/
	UartNs550Init(ZIGBEE_UART_NUM, UartZigbeeIntrHandler);

	/* 创建信号量 */
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	m_sem_zigbee_recv = Semaphore_create(0, &semParams, NULL);

	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	m_sem_zigbee_send = Semaphore_create(1, &semParams, NULL);

}
void ZigbeeSend(void * pData, int len)
{
	Semaphore_pend(m_sem_zigbee_send, BIOS_WAIT_FOREVER);
	UartNs550Send(ZIGBEE_UART_NUM, pData, len);
}

Void taskZigbee(UArg a0, UArg a1)
{
	UART550_BUFFER * Buff;
	int state = 0;
	int i;
	uint8_t c;
	p_msg_t pMsg;
	int commandLen = 0;

	ZigbeeInit();

	while(1)
	{
		Semaphore_pend(m_sem_zigbee_recv, BIOS_WAIT_FOREVER);

		Buff = UartNs550PopBuffer(ZIGBEE_UART_NUM);

		for(i = 0;i<Buff->Length;i++)
		{
			c = Buff->Buffer[i];
			switch(state)
			{
				case 0:  //等待包头 $
				{
					if('$' == c)
					{
						state = 1;
						commandLen = 0;
						pMsg = Message_getEmpty();
						pMsg->type = zigbee;
						memset(pMsg->data,0,MSGSIZE);
					}
					break;
				}
				case 1:  //接收命令
				{
					if( c == '^')
					{
						state = 0;

						pMsg->dataLen = commandLen;

						Message_post(pMsg);
					}
					else
					{
						pMsg->data[commandLen++] = c;
					}

					break;
				}


			} // switch(state)
		}

	} /*  while(1)  */
}

