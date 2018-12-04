/*
 * RFID_drv.c
 *
 *  Created on: 2018-11-14
 *      Author: zhtro
 */

#include "RFID_drv.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "uartns550.h"
#include "stdint.h"
#include <xdc/runtime/Log.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include "assert.h"




//macro
#define RFID_HEAD  0xBB
#define RFID_END1  0x0D
#define RFID_END2  0x0A
#define RFID_CMD_LOOPCHECK_EPC 0x17

#define RFID_REPONSE_LEN     128
#define RFID_TX_BUFFER_SIZE  128

typedef enum  {
	Init,
	Ready,
	Head,
	Type,
	Len,
	Data,
	CRC,
	CRCcorrect,
	END1,
	END2,
	Recv
}RFID_state;


typedef struct {
	RFID_state state;
	unsigned char type;
	unsigned char msgLen;
	unsigned char curLen;
	unsigned char msg[RFID_REPONSE_LEN];
	unsigned char crc;

} RFID_instance_t;



/****************************************************************************/
/*                                                                          */
/*              变量定义                                                        */
/*                                                                          */
/****************************************************************************/

//目前是单例
static uint16_t m_deviceNum = 0xFFFF;
static RFID_instance_t m_inst;
static RFIDcallback m_callback = 0;
static Semaphore_Handle sem_rfid_dataReady;

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                        */
/*                                                                          */
/****************************************************************************/
static void UartRFIDIntrHandler(void *CallBackRef, u32 Event, unsigned int EventData);
static void InitSem();


//utils function====================

static unsigned char calculateCRCAdd(unsigned char A, unsigned char B)
{
	return (unsigned char)(A + B)&(0xFF);
}
static unsigned char calculateCRC(unsigned char * data, int len)
{
	int sum = 0;
	int i;

	for (i = 0; i < len; i++)
	{
		sum += data[i];
	}
	return (unsigned char)(sum & 0xFF);
}


static int RFID_send_packet(unsigned char *data , int len)
{
	assert((len+4)<=RFID_TX_BUFFER_SIZE);

	unsigned char buffer[RFID_TX_BUFFER_SIZE];
	buffer[0] = 0xBB;
	memcpy(&buffer[1], data, len);
	buffer[len+1] = calculateCRC(data,len);
	buffer[len+2] = 0x0D;
	buffer[len+3] = 0x0A;


	UartNs550Send(m_deviceNum, &(buffer[0]), len+4);


	return 1;
}

static void InitSem()
{

	Semaphore_Params semParams;

	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	sem_rfid_dataReady = Semaphore_create(0, &semParams, NULL);
}

static void RFIDInstanceReset(RFID_instance_t * inst)
{
    inst->crc = 0;
    inst->curLen = 0;
    inst->msgLen = 0;
    inst->type = 0;
    inst->state = Ready;
}

//API
//-----------------------------//




void RFIDDeviceInit(uint16_t deviceNum)
{
	UartNs550Init(deviceNum,UartRFIDIntrHandler);
	InitSem();
	m_deviceNum = deviceNum;
	RFIDInstanceReset(&m_inst);

}
void RFIDRegisterReadCallBack(RFIDcallback cb)
{
	m_callback = cb;
}



//开始循环查询EPC
int RFIDStartLoopCheckEpc()
{
	unsigned char data[4];

	data[0] = 0x17;
	data[1] = 0x02;
	data[2] = 0x00;
	data[3] = 0x00;

	return RFID_send_packet(data, 4);
}

int RFIDStopLoopCheckEpc()
{
	unsigned char data[2];

	data[0] = 0x18;
	data[1] = 0x00;

	return RFID_send_packet(data,2);

}






void RFIDPendForData()
{
	Semaphore_pend(sem_rfid_dataReady, BIOS_WAIT_FOREVER);
}






//RFID 状态机
static RFID_state protocolStateMachine(uint8_t c,RFID_instance_t * inst)
{
	unsigned char calCRC;
	switch(inst->state)
	{
		case Ready:
			if(0xBB == c)
			{
				inst->state = Head;
			}
			break;
		case Head:
			inst->type = c;
			inst->state = Type;
			break;
		case Type:
			inst->msgLen = c;
			inst->state = Len;
			break;
		case Len:
			inst->msg[inst->curLen++] = c;
			if(inst->msgLen == inst->curLen)
			{
				inst->state = Data;
			}
			break;
		case Data:
			inst->crc = c;
			inst->state = CRC;
			break;
		case CRC:
			calCRC = calculateCRC(inst->msg, inst->msgLen);
			calCRC = calculateCRCAdd(calCRC, inst->type);
			calCRC = calculateCRCAdd(calCRC, inst->msgLen);
			if(inst->crc == calCRC)
			{
				if (0x0D == c)
				{
					inst->state = END1;
				}
				else {
				    RFIDInstanceReset(inst);
				}
			}
			else
			{
			    Log_warning2("RFID CRC error, recv %d ,calc %d\n",  inst->crc, calCRC);
			    RFIDInstanceReset(inst);

			}
			break;

		case END1:
			if(0x0A == c)
			{
				inst->state = END2;
			}
			else{
			    RFIDInstanceReset(inst);
			}
			break;

		case END2:
			if(m_callback==0)
			{
				Log_error0("RFID state machine error, need to register call back fucntion\n");
			}
			else{
				m_callback(inst->type, inst->msg, inst->msgLen);
			}
			RFIDInstanceReset(inst);
			break;

		default:
			Log_error1("RFID state machine error, unknown state: %d\n",  inst->state);
		



	}
	return inst->state;
}


void RFIDProcess()
{
	UART550_BUFFER * Buff;
	int i;

	Buff = UartNs550PopBuffer(m_deviceNum);

	for(i=0;i<Buff->Length;i++)
	{
        protocolStateMachine(Buff->Buffer[i], &m_inst );
    }


}

static void UartRFIDIntrHandler(void *CallBackRef, u32 Event, unsigned int EventData)
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
        Semaphore_post(sem_rfid_dataReady);
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_TIMEOUT) {
        NextBuffer = UartNs550PushBuffer(DeviceNum,EventData);
        UartNs550Recv(DeviceNum, NextBuffer, BUFFER_MAX_SIZE);
//		TotalReceivedCount = EventData;
		Semaphore_post(sem_rfid_dataReady);
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



