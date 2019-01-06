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

#define RFID_DEVICENUM  0  //TODO: 放入一个配置表中

extern fbdata_t fbData;

void rfid_callBack(uint16_t deviceNum, uint8_t type, uint8_t data[], uint32_t len )
{
	p_msg_t msg;


	switch(type)
	{
		case 0x97:  //循环查询EPC的返回  回传EPC第一个byte
			Log_info2("RFID[%d] EPC:\t%2X ", deviceNum,data[2]);
			//填充回传数据

			//memcpy(fbData.rfid, &(data[2]),12);  //epc 从第2字节开始，长度12字节
			fbData.rfid = data[2];
			fbData.rfidReadTime = Timestamp_get32();

			msg = Message_getEmpty();
			msg->type = rfid;
			msg->data[0] = data[2];
			msg->dataLen = 1;
			Message_post(msg);
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

	while(1)
	{

		RFIDPendForData(RFID_DEVICENUM);

		RFIDProcess(RFID_DEVICENUM);

	}

}



