#include "canModule.h"
#include "sja_common.h"
#include <xdc/runtime/Log.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>



#define CAN_DEV_0 0
static Semaphore_Handle sem_dataReady;

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
        Semaphore_post(sem_dataReady);
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
	sem_dataReady = Semaphore_create(0, &semParams, NULL);
}

static void testSja1000Task()
{
    CAN_DATA_OBJ *canRecvData;
    /*初始化信用量*/
    InitSem();
    /*初始化CAN设备表*/
    canTableInit();
    /*初始化CAN设备*/
    canOpen(CAN_DEV_0, CAN0IntrHandler, CAN_DEV_0);

    canRecvData->ID = 0x0;
    canRecvData->SendType = 0;
    canRecvData->RemoteFlag = 0;
    canRecvData->ExternFlag = 0;
    canRecvData->DataLen = 8;
    canRecvData->Data[0] = 'a';
    canRecvData->Data[1] = 'b';
    canRecvData->Data[2] = 'c';
    canRecvData->Data[3] = 'd';
    canRecvData->Data[4] = 'e';
    canRecvData->Data[5] = 'f';
    canRecvData->Data[6] = 'g';
    canRecvData->Data[7] = 'h';
    while(1)
    {
        
        Semaphore_pend(sem_dataReady, BIOS_WAIT_FOREVER);
        canRecvData = canPopBuffer(CAN_DEV_0);

        Log_info0("before send");
        canWrite(CAN_DEV_0, canRecvData);
        Log_info0("after send");
        //canRecvData->ID++;
        //canRecvData->SendType = 0;
        
        //canRead(CAN_DEV_0,canRecvData);
        //Task_sleep(10);

    }
}


void testCantaskInit()
{
	Task_Handle task;
	Error_Block eb;
	Task_Params taskParams;

	Error_init(&eb);
    Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
	task = Task_create(testSja1000Task, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}

