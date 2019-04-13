/*
 * RFID.c
 *
 *  管理RFID读写器
 *
 *  接收RFID数据，解析之后放入消息队列
 *
 *  Created on: 2018-12-3
 *      Author: zhtro
 */

#include <xdc/std.h>
#include "uartns550.h"
#include "xil_types.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include "Sensor/RFID/RFID_drv.h"
#include <xdc/runtime/Log.h>
#include "Message/Message.h"
#include <xdc/runtime/Timestamp.h>
#include "task_moto.h"
#include <ti/sysbios/knl/Clock.h>


#define RFID_DEVICENUM  0  //TODO: 放入一个配置表中

extern fbdata_t g_fbData;
static Clock_Handle clock_rfid_heart;
static uint8_t timeout_flag = 0;
static uint32_t circleNum = 0;
static uint8_t lastrfid;

static xdc_Void RFIDConnectionClosed(xdc_UArg arg)
{
    p_msg_t msg;  
    /*
    *TODO:添加断连处理，发送急停消息，进入急停模式，并设置ErrorCode
    */
    msg = Message_getEmpty();
	msg->type = error;
	msg->data[0] = ERROR_RFID_TIMEOUT;
	msg->dataLen = 1;
	Message_post(msg);
	timeout_flag = 1;
    
    //setErrorCode(ERROR_CONNECT_TIMEOUT);
}

static void InitTimer()
{
	Clock_Params clockParams;


	Clock_Params_init(&clockParams);
	clockParams.period = 0;       // one shot
	clockParams.startFlag = FALSE;

	clock_rfid_heart = Clock_create(RFIDConnectionClosed, 10000, &clockParams, NULL);
}


void rfid_callBack(uint16_t deviceNum, uint8_t type, uint8_t data[], uint32_t len )
{
	p_msg_t msg;

	switch(type)
	{
		case 0x97:  //循环查询EPC的返回  回传EPC第一个byte
			Log_info2("RFID[%d] EPC:\t%2X ", deviceNum,data[2]);
			//填充回传数据

			//memcpy(fbData.rfid, &(data[2]),12);  //epc 从第2字节开始，长度12字节
			g_fbData.rfid = data[2];
			g_fbData.rfidReadTime = Timestamp_get32();
			/*记录圈数*/
			if(data[2] != lastrfid && data[2] == 0x06)
			{
				circleNum++;
			}
			lastrfid = data[2];
			g_fbData.circleNum = circleNum;

			msg = Message_getEmpty();
			msg->type = rfid;
			msg->data[0] = data[2];
			msg->dataLen = 1;
			Message_post(msg);
			Clock_setTimeout(clock_rfid_heart,3000);
			Clock_start(clock_rfid_heart);
			break;
       case 0x40:
            Clock_setTimeout(clock_rfid_heart,3000);
            Clock_start(clock_rfid_heart);
            if(timeout_flag == 1)
            {
            	timeout_flag = 0;
            	RFIDStartLoopCheckEpc(RFID_DEVICENUM);
            }
            break;

	}
}

/****************************************************************************/
/*                                                                          */
/*              函数定义                                                        */
/*                                                                          */
/****************************************************************************/
Void taskRFID(UArg a0, UArg a1)
{

	RFIDDeviceOpen (RFID_DEVICENUM);
	RFIDRegisterReadCallBack(RFID_DEVICENUM, rfid_callBack);   //回调函数会在RFIDProcess里面调用

	RFIDStartLoopCheckEpc(RFID_DEVICENUM);
    InitTimer();
    Clock_start(clock_rfid_heart);

	while(1)
	{

		RFIDPendForData(RFID_DEVICENUM);

		RFIDProcess(RFID_DEVICENUM);

	}

}



