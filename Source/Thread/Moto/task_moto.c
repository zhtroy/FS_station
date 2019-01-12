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
#include "stdio.h"
#include <math.h>


/********************************************************************************/
/*          外部全局变量                                                              */
/********************************************************************************/

extern ctrlData g_carCtrlData;
#if 1
fbdata_t g_fbData;
extern uint8_t g_connectStatus;
#else 
motordata_t g_fbData.motorDataF, g_fbData.motorDataR;
static uint8_t g_connectStatus = 1;
#endif


float pidCalc(int16_t expRpm,int16_t realRpm,float kp,float ki, float ku,uint8_t clear);

/********************************************************************************/
/*          静态全局变量                                                              */
/********************************************************************************/
static Semaphore_Handle sem_dataReady;
static Semaphore_Handle sem_txReady;
static Semaphore_Handle sem_pid;

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

    sem_pid = Semaphore_create(1, &semParams, NULL);
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
        Semaphore_pend(sem_pid, 100);        //100ms超时发送油门

		canTx->Gear = g_carCtrlData.Gear;
        
		if (g_connectStatus)
		{
			canTx->ThrottleL = g_carCtrlData.Throttle;
			canTx->ThrottleH = 0;
		}
		else
		{
			canTx->ThrottleL = 0;
			canTx->ThrottleH = 0;
		}


		if ((g_carCtrlData.MotoSel == FRONT_ONLY) || (g_carCtrlData.MotoSel == FRONT_REAR))
		{
			canTx->Mode = 0x28;		                //前后电机方向相反
			canSendData.ID = MOTO_F_CANID1;
            //canSendData.DataLen = 8;
            Semaphore_pend(sem_txReady, BIOS_WAIT_FOREVER);
			canWrite(MOTO_CAN_DEVNUM, &canSendData);      
		}
        
		if ((g_carCtrlData.MotoSel == REAR_ONLY) || (g_carCtrlData.MotoSel == FRONT_REAR))
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
	//pid
    uint16_t recvRpm;
    uint16_t recvThrottle;
    float adjThrottle;
    float adjbrake;
    //uint8_t mode;
    static float hisThrottle = 0;

    CAN_DATA_OBJ *canRecvData;
	g_fbData.motorDataF.MotoId = MOTO_FRONT;
	g_fbData.motorDataR.MotoId = MOTO_REAR;
	
	while (1)
	{
        Semaphore_pend(sem_dataReady, BIOS_WAIT_FOREVER);
        canRecvData = canPopBuffer(MOTO_CAN_DEVNUM);
        
		switch ((canRecvData->ID) & 0x1fffffff)
		{
    		//Front电机反馈信息打包
    		case MOTO_F_CANID2:
    			g_fbData.motorDataF.Gear       = (uint8_t)canRecvData->Data[0];
    			g_fbData.motorDataF.ThrottleL  = (uint8_t)canRecvData->Data[1];
    			g_fbData.motorDataF.ThrottleH  = (uint8_t)canRecvData->Data[2];
    			g_fbData.motorDataF.MotoMode   = (uint8_t)canRecvData->Data[3];
    			g_fbData.motorDataF.RPML       = (uint8_t)canRecvData->Data[4];
    			g_fbData.motorDataF.RPMH       = (uint8_t)canRecvData->Data[5];
    			g_fbData.motorDataF.MotoTemp   = (uint8_t)canRecvData->Data[6] - 40;
    			g_fbData.motorDataF.DriverTemp = (uint8_t)canRecvData->Data[7] - 40;

    			//printf("m %d\n", carCtrlData.AutoMode);
    			//mode = carCtrlData.AutoMode;
    			if(2 == g_carCtrlData.AutoMode)
    			{
					recvRpm = (g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML;
					recvThrottle = (g_fbData.motorDataF.ThrottleH << 8) + g_fbData.motorDataF.ThrottleL;
					adjThrottle = pidCalc(g_carCtrlData.RPM,recvRpm,(float)(g_carCtrlData.KP/1000000.0),
											(float)(g_carCtrlData.KI/1000000.0),(float)(g_carCtrlData.KU/1000000.0), 0);

					if(hisThrottle < 0 && adjThrottle >0)
					{
						hisThrottle += 5.0*adjThrottle;
					}
					else
					{
						hisThrottle += adjThrottle;
					}

					if(hisThrottle > MAX_THROTTLE_SIZE)
						hisThrottle = MAX_THROTTLE_SIZE;
					else if(hisThrottle < MIN_THROTTLE_SIZE)
						hisThrottle = MIN_THROTTLE_SIZE;
					else;

					if(hisThrottle < 0)   //刹车状态
					{

						g_carCtrlData.Throttle = 0;

						if(hisThrottle < BREAK_THRESHOLD)
							adjbrake = - BRAKE_THRO_RATIO* (hisThrottle - BREAK_THRESHOLD);
						else
							adjbrake = 0;

						if(adjbrake > MAX_BRAKE_SIZE)
							g_carCtrlData.Brake = MAX_BRAKE_SIZE;
						else
							g_carCtrlData.Brake = round(adjbrake);

					}
					else  //油门
					{
						g_carCtrlData.Throttle = round(hisThrottle);
						g_carCtrlData.Brake = 0;
					}
    			}
    			else
    			{
    				hisThrottle = 0;
    				pidCalc(0,0,0,0,0,1);
    			}

    			Semaphore_post(sem_pid);


    			break;
    		case MOTO_F_CANID3:
    			g_fbData.motorDataF.VoltL      = (uint8_t)canRecvData->Data[0];
    			g_fbData.motorDataF.VoltH      = (uint8_t)canRecvData->Data[1];
    			g_fbData.motorDataF.CurrentL   = (uint8_t)canRecvData->Data[2];
    			g_fbData.motorDataF.CurrentH   = (uint8_t)canRecvData->Data[3];
    			g_fbData.motorDataF.DistanceL  = (uint8_t)canRecvData->Data[4];
    			g_fbData.motorDataF.DistanceH  = (uint8_t)canRecvData->Data[5];
    			g_fbData.motorDataF.ErrCodeL   = (uint8_t)canRecvData->Data[6];
    			g_fbData.motorDataF.ErrCodeH   = (uint8_t)canRecvData->Data[7];
    			break;
    		case MOTO_F_CANID4:
    			g_fbData.motorDataF.TorqueCtrlL    = (uint8_t)canRecvData->Data[0];
    			g_fbData.motorDataF.TorqueCtrlH    = (uint8_t)canRecvData->Data[1];
    			g_fbData.motorDataF.RPMCtrlL       = (uint8_t)canRecvData->Data[2];
    			g_fbData.motorDataF.RPMCtrlH       = (uint8_t)canRecvData->Data[3];
    			g_fbData.motorDataF.TorqueL        = (uint8_t)canRecvData->Data[4];
    			g_fbData.motorDataF.TorqueH        = (uint8_t)canRecvData->Data[5];
    			g_fbData.motorDataF.CanReserved1   = (uint8_t)canRecvData->Data[6];
    			g_fbData.motorDataF.CanReserved2   = (uint8_t)canRecvData->Data[7];
    			break;
    		//Front电机反馈信息打包
    		case MOTO_R_CANID2:
    			g_fbData.motorDataR.Gear           = (uint8_t)canRecvData->Data[0];
    			g_fbData.motorDataR.ThrottleL      = (uint8_t)canRecvData->Data[1];
    			g_fbData.motorDataR.ThrottleH      = (uint8_t)canRecvData->Data[2];
    			g_fbData.motorDataR.MotoMode       = (uint8_t)canRecvData->Data[3];
    			g_fbData.motorDataR.RPML           = (uint8_t)canRecvData->Data[4];
    			g_fbData.motorDataR.RPMH           = (uint8_t)canRecvData->Data[5];
    			g_fbData.motorDataR.MotoTemp       = (uint8_t)canRecvData->Data[6] - 40;
    			g_fbData.motorDataR.DriverTemp     = (uint8_t)canRecvData->Data[7] - 40;
    			break;
    		case MOTO_R_CANID3:
    			g_fbData.motorDataR.VoltL          = (uint8_t)canRecvData->Data[0];
    			g_fbData.motorDataR.VoltH          = (uint8_t)canRecvData->Data[1];
    			g_fbData.motorDataR.CurrentL       = (uint8_t)canRecvData->Data[2];
    			g_fbData.motorDataR.CurrentH       = (uint8_t)canRecvData->Data[3];
    			g_fbData.motorDataR.DistanceL      = (uint8_t)canRecvData->Data[4];
    			g_fbData.motorDataR.DistanceH      = (uint8_t)canRecvData->Data[5];
    			g_fbData.motorDataR.ErrCodeL       = (uint8_t)canRecvData->Data[6];
    			g_fbData.motorDataR.ErrCodeH       = (uint8_t)canRecvData->Data[7];
    			break;
    		case MOTO_R_CANID4:
    			g_fbData.motorDataR.TorqueCtrlL    = (uint8_t)canRecvData->Data[0];
    			g_fbData.motorDataR.TorqueCtrlH    = (uint8_t)canRecvData->Data[1];
    			g_fbData.motorDataR.RPMCtrlL       = (uint8_t)canRecvData->Data[2];
    			g_fbData.motorDataR.RPMCtrlH       = (uint8_t)canRecvData->Data[3];
    			g_fbData.motorDataR.TorqueL        = (uint8_t)canRecvData->Data[4];
    			g_fbData.motorDataR.TorqueH        = (uint8_t)canRecvData->Data[5];
    			g_fbData.motorDataR.CanReserved1   = (uint8_t)canRecvData->Data[6];
    			g_fbData.motorDataR.CanReserved2   = (uint8_t)canRecvData->Data[7];
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
		/*
		fbData.motorDataR.ThrottleL++;
		fbData.motorDataR.RPMH++;
		fbData.motorDataF.ThrottleH++;
		fbData.motorDataF.RPML++;
		*/
		g_fbData.brake = g_carCtrlData.Brake;
		g_fbData.railstate = getRailState();
		CellSendData((char*) &g_fbData, sizeof(g_fbData));

		Task_sleep(100);
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
	if (g_carCtrlData.MotoSel == FRONT_ONLY)
	{
		uCarRPM = (g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML;
	}
	else if (g_carCtrlData.MotoSel == REAR_ONLY)
	{
		uCarRPM = (g_fbData.motorDataR.RPMH << 8) + g_fbData.motorDataR.RPML;
	}
	else if (g_carCtrlData.MotoSel == FRONT_REAR)
	{
		uCarRPM = ((g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML + (g_fbData.motorDataR.RPMH << 8) + g_fbData.motorDataR.RPML) / 2;
	}

	return uCarRPM;
}
#if 0
float pidCalc(int16_t expRpm,int16_t realRpm,float kp,float ki, uint8_t clear)
{
    int32_t diffRpm;
    float adjThrottle;
    static float lastThrottle = 0;

    if(clear==1)
    {
    	lastThrottle =0;
    	return 0;
    }

    diffRpm = expRpm - realRpm;

    /*限定差值最大范围*/
    if(diffRpm > DIFF_RPM_UPSCALE)
        diffRpm = DIFF_RPM_UPSCALE;
    else if(diffRpm < DIFF_RPM_DWSCALE)
        diffRpm = DIFF_RPM_DWSCALE;
    else;


    adjThrottle = kp*diffRpm + ki*lastThrottle;


    if(adjThrottle > MAX_THROTTLE_SIZE)
        adjThrottle = MAX_THROTTLE_SIZE;
    else if(adjThrottle < MIN_THROTTLE_SIZE)
        adjThrottle = MIN_THROTTLE_SIZE;
    else;

    lastThrottle = adjThrottle;

    return adjThrottle;
}
#endif

float pidCalc(int16_t expRpm,int16_t realRpm,float kp,float ki, float ku,uint8_t clear)
{
    int32_t diffRpm;
    float adjThrottle;
    static int32_t lastDiff = 0;
    static int32_t elastDiff = 0;

    if(clear==1)
    {
    	lastDiff =0;
    	elastDiff = 0;
    	return 0;
    }

    diffRpm = expRpm - realRpm;

    //限定差值最大范围
    if(diffRpm > DIFF_RPM_UPSCALE)
        diffRpm = DIFF_RPM_UPSCALE;
    else if(diffRpm < DIFF_RPM_DWSCALE)
        diffRpm = DIFF_RPM_DWSCALE;
    else;

    adjThrottle = kp*(diffRpm - lastDiff) + ki*diffRpm + ku*(diffRpm - 2*lastDiff + elastDiff);

    elastDiff = lastDiff;
    lastDiff = diffRpm;

    if(adjThrottle > ADJ_THROTTLE_UPSCALE)
        adjThrottle = ADJ_THROTTLE_UPSCALE;
    else if(adjThrottle < ADJ_THROTTLE_DWSCALE)
        adjThrottle = ADJ_THROTTLE_DWSCALE;
    else;

    return adjThrottle;
}


