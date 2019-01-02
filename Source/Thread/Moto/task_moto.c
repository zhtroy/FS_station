#include "canModule.h"
#include "sja_common.h"
#include "task_moto.h"
#include "task_ctrldata.h"
#include "Sensor/CellCommunication/CellCommunication.h"

/*SYSBIOS includes*/
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>


/********************************************************************************/
/*          外部全局变量                                                              */
/********************************************************************************/

extern ctrlData carCtrlData;
#if 1
fbdata_t fbData;
extern uint8_t connectStatus;
#else 
motordata_t fbData.motorDataF, fbData.motorDataR;
static uint8_t connectStatus = 1;
#endif


/********************************************************************************/
/*          静态全局变量                                                              */
/********************************************************************************/
static Semaphore_Handle sem_dataReady;
static Semaphore_Handle sem_txReady;

static CAN_DATA_OBJ canSendData;
    

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
        Semaphore_post(sem_txReady);
    }

}

static void InitSem()
{

	Semaphore_Params semParams;
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	sem_dataReady = Semaphore_create(0, &semParams, NULL);

    sem_txReady = Semaphore_create(1, &semParams, NULL);
}

static void motoSendTask(void)
{
    canData *canTx = (canData *)canSendData.Data;

    /*初始化帧类型*/
    canSendData.ID = 0;
    canSendData.SendType = 0;
    canSendData.RemoteFlag = 0;
    canSendData.ExternFlag = 1;
    canSendData.DataLen = 8;
    
    /*初始化电机控制数据*/
    canTx->Gear = 0x00;         //空挡
    canTx->ThrottleL = 0x00;
    canTx->ThrottleH = 0x00;
    canTx->Mode = 0x20;         //油门，前进
    canTx->TorqueL = 0x00;
    canTx->TorqueH = 0x00;
    canTx->SpeedOrBreakL = 0x00;
    canTx->SpeedOrBreakH = 0x00;
    
	while (1)
	{
		canTx->Gear = carCtrlData.Gear;
        
		if (connectStatus)
		{
			canTx->ThrottleL = carCtrlData.Throttle;
			canTx->ThrottleH = 0;
		}
		else
		{
			canTx->ThrottleL = 0;
			canTx->ThrottleH = 0;
		}


		if ((carCtrlData.MotoSel == FRONT_ONLY) || (carCtrlData.MotoSel == FRONT_REAR))
		{
			canTx->Mode = 0x28;		                //前后电机方向相反
			canSendData.ID = MOTO_F_CANID1;
            //canSendData.DataLen = 8;
            Semaphore_pend(sem_txReady, BIOS_WAIT_FOREVER);
			canWrite(MOTO_CAN_DEVNUM, &canSendData);      
		}
        
		if ((carCtrlData.MotoSel == REAR_ONLY) || (carCtrlData.MotoSel == FRONT_REAR))
		{
			canTx->Mode = 0x20;		                //前后电机方向相反
			canSendData.ID = MOTO_R_CANID1;
            //canSendData.DataLen = 8;
            Semaphore_pend(sem_txReady, BIOS_WAIT_FOREVER);
			canWrite(MOTO_CAN_DEVNUM, &canSendData);      
		}

		Task_sleep(50);
	}
}

static void motoRecvTask(void)
{
    CAN_DATA_OBJ *canRecvData;
	fbData.motorDataF.MotoId = MOTO_FRONT;
	fbData.motorDataR.MotoId = MOTO_REAR;
	
	while (1)
	{
        Semaphore_pend(sem_dataReady, BIOS_WAIT_FOREVER);
        canRecvData = canPopBuffer(MOTO_CAN_DEVNUM);
        
		switch ((canRecvData->ID) & 0x1fffffff)
		{
    		//Front电机反馈信息打包
    		case MOTO_F_CANID2:
    			fbData.motorDataF.Gear       = (uint8_t)canRecvData->Data[0];
    			fbData.motorDataF.ThrottleL  = (uint8_t)canRecvData->Data[1];
    			fbData.motorDataF.ThrottleH  = (uint8_t)canRecvData->Data[2];
    			fbData.motorDataF.MotoMode   = (uint8_t)canRecvData->Data[3];
    			fbData.motorDataF.RPML       = (uint8_t)canRecvData->Data[4];
    			fbData.motorDataF.RPMH       = (uint8_t)canRecvData->Data[5];
    			fbData.motorDataF.MotoTemp   = (uint8_t)canRecvData->Data[6] - 40;
    			fbData.motorDataF.DriverTemp = (uint8_t)canRecvData->Data[7] - 40;
    			break;
    		case MOTO_F_CANID3:
    			fbData.motorDataF.VoltL      = (uint8_t)canRecvData->Data[0];
    			fbData.motorDataF.VoltH      = (uint8_t)canRecvData->Data[1];
    			fbData.motorDataF.CurrentL   = (uint8_t)canRecvData->Data[2];
    			fbData.motorDataF.CurrentH   = (uint8_t)canRecvData->Data[3];
    			fbData.motorDataF.DistanceL  = (uint8_t)canRecvData->Data[4];
    			fbData.motorDataF.DistanceH  = (uint8_t)canRecvData->Data[5];
    			fbData.motorDataF.ErrCodeL   = (uint8_t)canRecvData->Data[6];
    			fbData.motorDataF.ErrCodeH   = (uint8_t)canRecvData->Data[7];
    			break;
    		case MOTO_F_CANID4:
    			fbData.motorDataF.TorqueCtrlL    = (uint8_t)canRecvData->Data[0];
    			fbData.motorDataF.TorqueCtrlH    = (uint8_t)canRecvData->Data[1];
    			fbData.motorDataF.RPMCtrlL       = (uint8_t)canRecvData->Data[2];
    			fbData.motorDataF.RPMCtrlH       = (uint8_t)canRecvData->Data[3];
    			fbData.motorDataF.TorqueL        = (uint8_t)canRecvData->Data[4];
    			fbData.motorDataF.TorqueH        = (uint8_t)canRecvData->Data[5];
    			fbData.motorDataF.CanReserved1   = (uint8_t)canRecvData->Data[6];
    			fbData.motorDataF.CanReserved2   = (uint8_t)canRecvData->Data[7];
    			break;
    		//Front电机反馈信息打包
    		case MOTO_R_CANID2:
    			fbData.motorDataR.Gear           = (uint8_t)canRecvData->Data[0];
    			fbData.motorDataR.ThrottleL      = (uint8_t)canRecvData->Data[1];
    			fbData.motorDataR.ThrottleH      = (uint8_t)canRecvData->Data[2];
    			fbData.motorDataR.MotoMode       = (uint8_t)canRecvData->Data[3];
    			fbData.motorDataR.RPML           = (uint8_t)canRecvData->Data[4];
    			fbData.motorDataR.RPMH           = (uint8_t)canRecvData->Data[5];
    			fbData.motorDataR.MotoTemp       = (uint8_t)canRecvData->Data[6] - 40;
    			fbData.motorDataR.DriverTemp     = (uint8_t)canRecvData->Data[7] - 40;
    			break;
    		case MOTO_R_CANID3:
    			fbData.motorDataR.VoltL          = (uint8_t)canRecvData->Data[0];
    			fbData.motorDataR.VoltH          = (uint8_t)canRecvData->Data[1];
    			fbData.motorDataR.CurrentL       = (uint8_t)canRecvData->Data[2];
    			fbData.motorDataR.CurrentH       = (uint8_t)canRecvData->Data[3];
    			fbData.motorDataR.DistanceL      = (uint8_t)canRecvData->Data[4];
    			fbData.motorDataR.DistanceH      = (uint8_t)canRecvData->Data[5];
    			fbData.motorDataR.ErrCodeL       = (uint8_t)canRecvData->Data[6];
    			fbData.motorDataR.ErrCodeH       = (uint8_t)canRecvData->Data[7];
    			break;
    		case MOTO_R_CANID4:
    			fbData.motorDataR.TorqueCtrlL    = (uint8_t)canRecvData->Data[0];
    			fbData.motorDataR.TorqueCtrlH    = (uint8_t)canRecvData->Data[1];
    			fbData.motorDataR.RPMCtrlL       = (uint8_t)canRecvData->Data[2];
    			fbData.motorDataR.RPMCtrlH       = (uint8_t)canRecvData->Data[3];
    			fbData.motorDataR.TorqueL        = (uint8_t)canRecvData->Data[4];
    			fbData.motorDataR.TorqueH        = (uint8_t)canRecvData->Data[5];
    			fbData.motorDataR.CanReserved1   = (uint8_t)canRecvData->Data[6];
    			fbData.motorDataR.CanReserved2   = (uint8_t)canRecvData->Data[7];
    			break;
    		default:
    			break;
		}
	}
}
//TODO: 发送机车状态到4G
void taskMotoSendFdbkToCell()
{

	while(1){
		fbData.motorDataR.ThrottleL++;
		fbData.motorDataR.RPMH++;
		fbData.motorDataF.ThrottleH++;
		fbData.motorDataF.RPML++;

		CellSendData((char*) &fbData, sizeof(fbData));

		Task_sleep(200);
	}
}
void testMototaskInit()
{
	Task_Handle task;
	Task_Params taskParams;

    /*初始化信用量*/
    InitSem();
    /*初始化CAN设备表*/
    //canTableInit();
    /*初始化CAN设备*/
    canOpen(CAN_DEV_0, CAN0IntrHandler, CAN_DEV_0);
    
    Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
    
	task = Task_create(motoSendTask, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

    task = Task_create(motoRecvTask, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

    task = Task_create(taskMotoSendFdbkToCell, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}

uint16_t getRPM(void)
{
	uint16_t uCarRPM;
	if (carCtrlData.MotoSel == FRONT_ONLY)
	{
		uCarRPM = (fbData.motorDataF.RPMH << 8) + fbData.motorDataF.RPML;
	}
	else if (carCtrlData.MotoSel == REAR_ONLY)
	{
		uCarRPM = (fbData.motorDataR.RPMH << 8) + fbData.motorDataR.RPML;
	}
	else if (carCtrlData.MotoSel == FRONT_REAR)
	{
		uCarRPM = ((fbData.motorDataF.RPMH << 8) + fbData.motorDataF.RPML + (fbData.motorDataR.RPMH << 8) + fbData.motorDataR.RPML) / 2;
	}

	return uCarRPM;
}
