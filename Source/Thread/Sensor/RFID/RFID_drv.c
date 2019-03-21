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
#include "common.h"



#define RFID_MBOX_DEPTH (16)
/****************************************************************************/
/*                                                                          */
/*              变量定义                                                        */
/*                                                                          */
/****************************************************************************/

//RFID配置表
static RFID_instance_t rfid_cfg_table[]={
		//RFID设备0
		{
		  //串口设备号,不能重复
		  0
		},
		//RFID设备1
		{
		  //串口设备号,不能重复
		  1
		}

};

static uartDataObj_t rfidUartDataObj;

/****************************************************************************/
/*                                                                          */
/*              函数声明                                                        */
/*                                                                          */
/****************************************************************************/
static void uartRFIDIntrHandler(void *callBackRef, u32 event, unsigned int eventData);
static RFID_instance_t * getInstanceByDeviceNum(uint16_t deviceNum);


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


static int RFID_send_packet(uint16_t deviceNum, unsigned char *data , int len)
{

	assert((len+4)<=RFID_TX_BUFFER_SIZE);
	RFID_instance_t * pinst;

	pinst =getInstanceByDeviceNum(deviceNum);



	unsigned char buffer[RFID_TX_BUFFER_SIZE];
	buffer[0] = 0xBB;
	memcpy(&buffer[1], data, len);
	buffer[len+1] = calculateCRC(data,len);
	buffer[len+2] = 0x0D;
	buffer[len+3] = 0x0A;


	UartNs550Send(pinst->uartDeviceNum, &(buffer[0]), len+4);


	return 1;
}



static void RFIDstateMachineReset(RFID_stateMachine_t * inst)
{
    inst->crc = 0;
    inst->curLen = 0;
    inst->msgLen = 0;
    inst->type = 0;
    inst->state = Ready;
}


static RFID_instance_t * getInstanceByDeviceNum(uint16_t deviceNum)
{
	RFID_instance_t * pinst;
	int rfid_cfg_num;

	rfid_cfg_num = sizeof(rfid_cfg_table)/ sizeof (rfid_cfg_table[0]);

	if(deviceNum>=rfid_cfg_num){
		Log_error2("RFID: requested deviceNum [%d], total devices [%d]",deviceNum, rfid_cfg_num);
		BIOS_exit(0);
	}

	pinst = &(rfid_cfg_table[deviceNum]);

	return pinst;
}

// 根据UART devicenum 获取RFID num
static uint16_t getRFIDnumByUartNum(uint16_t uartNum)
{
	uint16_t i;
	RFID_instance_t * pinst;
	int rfid_cfg_num;

	rfid_cfg_num = sizeof(rfid_cfg_table)/ sizeof (rfid_cfg_table[0]);

	for(i=0;i<rfid_cfg_num;i++){
		if(rfid_cfg_table[i].uartDeviceNum ==  uartNum)
		{
			return i;
		}
	}
	return 0xFFFF;

}

//API
//-----------------------------//





void RFIDDeviceOpen(uint16_t deviceNum)
{
	RFID_instance_t * pinst;
	//Semaphore_Params semParams;
	Mailbox_Params mboxParams; 

	pinst = getInstanceByDeviceNum(deviceNum);
	//连接串口
	UartNs550Init(pinst->uartDeviceNum,uartRFIDIntrHandler);

    /* 初始化接收邮箱 */
    Mailbox_Params_init(&mboxParams);
    pinst->recvMbox = Mailbox_create (sizeof (uartDataObj_t),RFID_MBOX_DEPTH, &mboxParams, NULL);

	RFIDstateMachineReset(&(pinst->sminst));
    
    UartNs550Recv(deviceNum, &rfidUartDataObj.buffer, UART_REC_BUFFER_SIZE);
}


void RFIDRegisterReadCallBack(uint16_t deviceNum, RFIDcallback cb)
{
	RFID_instance_t * pinst;

	pinst = getInstanceByDeviceNum(deviceNum);
	pinst->callback = cb;

}



//开始循环查询EPC
int RFIDStartLoopCheckEpc(uint16_t deviceNum)
{
	unsigned char data[4];

	data[0] = 0x17;
	data[1] = 0x02;
	data[2] = 0x00;
	data[3] = 0x00;

	return RFID_send_packet(deviceNum, data, 4);
}

int RFIDStopLoopCheckEpc(uint16_t deviceNum)
{
	unsigned char data[2];

	data[0] = 0x18;
	data[1] = 0x00;

	return RFID_send_packet(deviceNum,data,2);

}




//RFID 状态机
static RFID_state protocolStateMachine(uint8_t c,uint16_t deviceNum)
{

	RFID_instance_t * pinst;
	RFID_stateMachine_t *inst;
	unsigned char calCRC;

	pinst =getInstanceByDeviceNum(deviceNum);
	inst = &(pinst->sminst);

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
				    RFIDstateMachineReset(inst);
				}
			}
			else
			{
			    Log_warning2("RFID CRC error, recv %d ,calc %d\n",  inst->crc, calCRC);
			    RFIDstateMachineReset(inst);

			}
			break;

		case END1:
			if(0x0A == c)
			{
				inst->state = END2;
			}
			else{
			    RFIDstateMachineReset(inst);
			}
			break;

		case END2:
			if(pinst->callback==0)
			{
				Log_error0("RFID state machine error, need to register call back fucntion\n");
			}
			else{
				pinst->callback(deviceNum, inst->type, inst->msg, inst->msgLen);
			}
			RFIDstateMachineReset(inst);
			break;

		default:
			Log_error1("RFID state machine error, unknown state: %d\n",  inst->state);
		



	}
	return inst->state;
}


void RFIDProcess(uint16_t deviceNum)
{
	int i;
	RFID_instance_t * pinst;
    uartDataObj_t uartDataObj;

	pinst =getInstanceByDeviceNum(deviceNum);

    Mailbox_pend(pinst->recvMbox, (Ptr *)&uartDataObj, BIOS_WAIT_FOREVER);

	for(i=0;i<uartDataObj.length;i++)
	{
        protocolStateMachine(uartDataObj.buffer[i], deviceNum );
    }


}

static void uartRFIDIntrHandler(void *callBackRef, u32 event, unsigned int eventData)
{
	u8 Errors;
	u16 UartDeviceNum = *((u16 *)callBackRef);
	u16 RFIDDeviceNum;
    u8 *NextBuffer;
    RFID_instance_t * pinst;


    RFIDDeviceNum = getRFIDnumByUartNum(UartDeviceNum);

    pinst =getInstanceByDeviceNum(RFIDDeviceNum);

	if (event == XUN_EVENT_SENT_DATA) {

	}

	if (event == XUN_EVENT_RECV_DATA || event == XUN_EVENT_RECV_TIMEOUT) {
        rfidUartDataObj.length = eventData;
        Mailbox_post(pinst->recvMbox, (Ptr *)&rfidUartDataObj, BIOS_NO_WAIT);
        UartNs550Recv(UartDeviceNum, &rfidUartDataObj.buffer, UART_REC_BUFFER_SIZE);
	}

	if (event == XUN_EVENT_RECV_ERROR) {
		Errors = UartNs550GetLastErrors(UartDeviceNum);
	}
}



