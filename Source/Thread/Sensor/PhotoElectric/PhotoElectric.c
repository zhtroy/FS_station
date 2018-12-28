/*
 * 光电传感器
 *
 * 所有光电对管独占一条CAN总线
 *
 */
#include "canModule.h"
#include "sja_common.h"
#include <xdc/runtime/Log.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <stdio.h>
#include "uartStdio.h"
#include "Sensor/PhotoElectric/PhotoElectric.h"


static Semaphore_Handle sem_photoelectric_dataReady;



/********************************************************************************/
/*          外部CAN0的用户中断Handler                                                   */
/*                                                                              */
/********************************************************************************/
static void CAN0IntrHandler(INT32 devsNum,INT32 event)
{
    CAN_DATA_OBJ *CurrntBuffer;
	/*
	 * a Frame has been received.
	 */
	if (event == 1) {
        CurrntBuffer = canPushBuffer(devsNum);
        canRead(devsNum, CurrntBuffer);
        Semaphore_post(sem_photoelectric_dataReady);
	}

    if (event == 2) {
        /* 发送中断 */
    }

}

static void InitSem()
{

	Semaphore_Params semParams;
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	sem_photoelectric_dataReady = Semaphore_create(0, &semParams, NULL);
}

//解析并处理光电对管数据
static photo_t PhotoEle_resolveCANdata(CAN_DATA_OBJ * pdata)
{
	photo_data_t * pphoto;
	photo_t p;

	pphoto = (photo_data_t*) (pdata->Data);

	p.deviceNum = pdata->ID;
	memcpy(&(p.content), pphoto, sizeof(photo_data_t));

	return p;
}

static void taskPhotoElectric()
{
    CAN_DATA_OBJ *canRecvData;
    photo_t photo;

    /*初始化信用量*/
    InitSem();
    /*初始化CAN设备表*/
    canTableInit();
    /*初始化CAN设备*/
    canOpen(PHOTO_CAN_DEV, CAN0IntrHandler, PHOTO_CAN_DEV);

    while(1)
    { 
        Semaphore_pend(sem_photoelectric_dataReady, BIOS_WAIT_FOREVER);
        canRecvData = canPopBuffer(PHOTO_CAN_DEV);

        photo = PhotoEle_resolveCANdata(canRecvData);

        /*
         * 串口输出，调试用
         */

        UARTprintf("photo deviceNum = %d\t",photo.deviceNum);
        if(photo.content.cmd ==CAN_CMD_RISE ){
			UARTPuts("rise: ",-1);
		}
		else if(photo.content.cmd == CAN_CMD_FALL){
			UARTPuts("fall: ",-1);
		}
		else{
			UARTPuts("unknown: ",-1);
		}

		UARTprintf("time: %d ms\tchn:%d\n", photo.content.data,photo.content.chn);



		/*
		 * 调试用 end
		 */


    }
}


/*
 *   API
 */

//FIXME: 目前arm板卡在上电后只能改一次CAN ID
void PhotoEle_changeID(uint32_t oldID, uint32_t newID)
{
	CAN_DATA_OBJ can_obj;
	photo_data_t * pphoto;

	can_obj.ID = oldID;
	can_obj.SendType = 0;
	can_obj.DataLen = 8;

	pphoto = (photo_data_t *) (can_obj.Data);
	pphoto->cmd = CAN_CMD_ID;
	pphoto->chn  = 0;
	*(uint32_t*)(pphoto->data) = newID;

	canWrite(PHOTO_CAN_DEV, &can_obj);
}

void PhotoEle_setLight(uint32_t id, uint8_t mask)
{
	CAN_DATA_OBJ can_obj;
	photo_data_t * pphoto;

	can_obj.ID = id;
	can_obj.SendType = 0;
	can_obj.DataLen = 8;

	pphoto = (photo_data_t *) (can_obj.Data);
	pphoto->cmd = CAN_CMD_CTRL;
	pphoto->chn  = mask;

	canWrite(PHOTO_CAN_DEV, &can_obj);

}

/*
 * 启动接收线程
 */
void PhotoEle_init()
{
	Task_Handle task;
	Error_Block eb;
	Task_Params taskParams;

	Error_init(&eb);
    Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
	task = Task_create(taskPhotoElectric, &taskParams, &eb);

	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}
