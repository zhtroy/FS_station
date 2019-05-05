#include "canModule.h"
#include "task_moto.h"
#include "Sensor/CellCommunication/CellCommunication.h"

/*SYSBIOS includes*/
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include "Message/Message.h"
#include "stdio.h"
#include <math.h>
#include "common.h"
#include "task_brake_servo.h"
#include "Parameter.h"
#include "Zigbee/Zigbee.h"

/* 宏定义 */
#define RX_MBOX_DEPTH (32)
#define MOTO_CONNECT_TIMEOUT (300)

/********************************************************************************/
/*          外部全局变量                                                              */
/********************************************************************************/

#if 1
fbdata_t g_fbData;
extern uint8_t g_connectStatus;
#else 
motordata_t g_fbData.motorDataF, g_fbData.motorDataR;
static uint8_t g_connectStatus = 1;
#endif
static Clock_Handle clockMotoRearHeart;
static Clock_Handle clockMotoFrontHeart;

static float MotoPidCalc(int16_t expRpm,int16_t realRpm,float kp,float ki, float ku,uint8_t clear);

static uint8_t frontValid = 0;
static uint8_t rearValid = 0;
/********************************************************************************/
/*          静态全局变量                                                              */
/********************************************************************************/
static Semaphore_Handle txReadySem;
static Semaphore_Handle pidSem;
static Mailbox_Handle rxDataMbox = NULL;

static canDataObj_t canSendData;

/*电机控制量*/
static moto_ctrl_t m_motoCtrl = {
		.MotoSel = FRONT_REAR,
		.ControlMode = MODE_THROTTLE,
		.Gear = GEAR_NONE,
		.Throttle = 0,
		.GoalRPM=0,
		.PidOn = 0
};


/********************************************************************************/
/*          外部CAN0的用户中断Handler                                                   */
/*                                                                              */
/********************************************************************************/
static void MotoCanIntrHandler(int32_t devsNum,int32_t event)
{
    canDataObj_t rxData;
	
	if (event == 1)         /* 收到一帧数据 */ 
    {
        CanRead(devsNum, &rxData);
        Mailbox_post(rxDataMbox, (Ptr *)&rxData, BIOS_NO_WAIT);
	}
    else if (event == 2)    /* 一帧数据发送完成 */ 
    {
        /* 发送中断 */
        Semaphore_post(txReadySem);
    }

}

static void MotoInitSem()
{
	Semaphore_Params semParams;
    Mailbox_Params mboxParams;

    /* 初始化发送和PID信用量 */
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
    txReadySem = Semaphore_create(1, &semParams, NULL);
    pidSem = Semaphore_create(1, &semParams, NULL);

    /* 初始化接收邮箱 */
    Mailbox_Params_init(&mboxParams);
    rxDataMbox = Mailbox_create (sizeof (canDataObj_t),RX_MBOX_DEPTH, &mboxParams, NULL);
    
}
static xdc_Void MotoRearConnectClosed(xdc_UArg arg)
{
    p_msg_t msg;
    /*
    *TODO:添加后轮断连处理，发送错误码消息
    */
    msg = Message_getEmpty();
	msg->type = error;
	msg->data[0] = ERROR_MOTOR_TIMEOUT;
	msg->dataLen = 1;
	Message_post(msg);
	//LogMsg("Moto Rear Connect Failed!!\r\n");
	rearValid = 0;
}

static xdc_Void MotoFrontConnectClosed(xdc_UArg arg)
{
    p_msg_t msg;
    /*
    *TODO:添加前轮断连处理，发送错误码消息
    */
    msg = Message_getEmpty();
	msg->type = error;
	msg->data[0] = ERROR_MOTOF_TIMEOUT;
	msg->dataLen = 1;
	Message_post(msg);
	frontValid = 0;
	//LogMsg("Moto Front Connect Failed!!\r\n");
}


static void MotoInitTimer()
{
	Clock_Params clockParams;
	Clock_Params_init(&clockParams);
	clockParams.period = 0;       // one shot
	clockParams.startFlag = FALSE;
	clockMotoRearHeart = Clock_create(MotoRearConnectClosed, 3000, &clockParams, NULL);
	clockMotoFrontHeart = Clock_create(MotoFrontConnectClosed, 3000, &clockParams, NULL);
	Clock_start(clockMotoRearHeart);
	Clock_start(clockMotoFrontHeart);
}

static void MotoSendTask(void)
{
    canData_t *canTx = (canData_t *)canSendData.Data;

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
        Semaphore_pend(pidSem, 100);        //100ms超时发送油门

		canTx->Gear = MotoGetGear();
        
		if (g_connectStatus)
		{
			canTx->ThrottleL = MotoGetThrottle();
			canTx->ThrottleH = 0;
		}
		else
		{
			canTx->ThrottleL = 0;
			canTx->ThrottleH = 0;
		}


		if ((MotoGetMotoSel() == FRONT_ONLY) || (MotoGetMotoSel() == FRONT_REAR))
		{
			canTx->Mode = 0x28;		                //前后电机方向相反
			canSendData.ID = MOTO_F_CANID1;
            //canSendData.DataLen = 8;
            Semaphore_pend(txReadySem, BIOS_WAIT_FOREVER);
			CanWrite(MOTO_CAN_DEVNUM, &canSendData);      
		}
        
		if ((MotoGetMotoSel() == REAR_ONLY) || (MotoGetMotoSel() == FRONT_REAR))
		{
			canTx->Mode = 0x20;		                //前后电机方向相反
			canSendData.ID = MOTO_R_CANID1;
            //canSendData.DataLen = 8;
            Semaphore_pend(txReadySem, BIOS_WAIT_FOREVER);
			CanWrite(MOTO_CAN_DEVNUM, &canSendData);      
		}

		
	}/* end while(1) */
}

static void MotoRecvTask(void)
{
	//pid
    uint16_t recvRpm;
    uint16_t recvThrottle;
    float adjThrottle;
    float adjbrake;
    //uint8_t mode;
    static float hisThrottle = 0;
    uint16_t lastRpm;
    uint8_t lastH;
    uint8_t lastL;
    uint8_t data_error = 0;
    uint16_t calcRpm = 0;
    uint32_t canID;
    uint32_t maxThrottle = 0;

    canDataObj_t canRecvData;
	g_fbData.motorDataF.MotoId = MOTO_FRONT;
	g_fbData.motorDataR.MotoId = MOTO_REAR;
	
	while (1)
	{
        Mailbox_pend(rxDataMbox, (Ptr *)&canRecvData, BIOS_WAIT_FOREVER);
        
        canID = (canRecvData.ID) & 0x1fffffff;
		switch (canID)
		{
    		//Front电机反馈信息打包
    		case MOTO_F_CANID2:
    			g_fbData.motorDataF.Gear       = (uint8_t)canRecvData.Data[0];
    			g_fbData.motorDataF.ThrottleL  = (uint8_t)canRecvData.Data[1];
    			g_fbData.motorDataF.ThrottleH  = (uint8_t)canRecvData.Data[2];
    			g_fbData.motorDataF.MotoMode   = (uint8_t)canRecvData.Data[3];
    			g_fbData.motorDataF.RPML       = (uint8_t)canRecvData.Data[4];
    			g_fbData.motorDataF.RPMH       = (uint8_t)canRecvData.Data[5];
    			g_fbData.motorDataF.MotoTemp   = (uint8_t)canRecvData.Data[6] - 40;
    			g_fbData.motorDataF.DriverTemp = (uint8_t)canRecvData.Data[7] - 40;
    			break;
    		case MOTO_F_CANID3:
    			g_fbData.motorDataF.VoltL      = (uint8_t)canRecvData.Data[0];
    			g_fbData.motorDataF.VoltH      = (uint8_t)canRecvData.Data[1];
    			g_fbData.motorDataF.CurrentL   = (uint8_t)canRecvData.Data[2];
    			g_fbData.motorDataF.CurrentH   = (uint8_t)canRecvData.Data[3];
    			g_fbData.motorDataF.DistanceL  = (uint8_t)canRecvData.Data[4];
    			g_fbData.motorDataF.DistanceH  = (uint8_t)canRecvData.Data[5];
    			g_fbData.motorDataF.ErrCodeL   = (uint8_t)canRecvData.Data[6];
    			g_fbData.motorDataF.ErrCodeH   = (uint8_t)canRecvData.Data[7];
    			/* 收到心跳，重启定时器 */
				Clock_setTimeout(clockMotoFrontHeart,MOTO_CONNECT_TIMEOUT);
				Clock_start(clockMotoFrontHeart);

				if(g_fbData.motorDataF.ErrCodeL == 0 && g_fbData.motorDataF.ErrCodeH == 0)
					frontValid = 1;
				else
					frontValid = 0;
    			break;
    		case MOTO_F_CANID4:
    			g_fbData.motorDataF.TorqueCtrlL    = (uint8_t)canRecvData.Data[0];
    			g_fbData.motorDataF.TorqueCtrlH    = (uint8_t)canRecvData.Data[1];
    			g_fbData.motorDataF.RPMCtrlL       = (uint8_t)canRecvData.Data[2];
    			g_fbData.motorDataF.RPMCtrlH       = (uint8_t)canRecvData.Data[3];
    			g_fbData.motorDataF.TorqueL        = (uint8_t)canRecvData.Data[4];
    			g_fbData.motorDataF.TorqueH        = (uint8_t)canRecvData.Data[5];
    			g_fbData.motorDataF.CanReserved1   = (uint8_t)canRecvData.Data[6];
    			g_fbData.motorDataF.CanReserved2   = (uint8_t)canRecvData.Data[7];
    			break;
    		//Front电机反馈信息打包
    		case MOTO_R_CANID2:
    			g_fbData.motorDataR.Gear           = (uint8_t)canRecvData.Data[0];
    			g_fbData.motorDataR.ThrottleL      = (uint8_t)canRecvData.Data[1];
    			g_fbData.motorDataR.ThrottleH      = (uint8_t)canRecvData.Data[2];
    			g_fbData.motorDataR.MotoMode       = (uint8_t)canRecvData.Data[3];
    			g_fbData.motorDataR.RPML           = (uint8_t)canRecvData.Data[4];
    			g_fbData.motorDataR.RPMH           = (uint8_t)canRecvData.Data[5];
    			g_fbData.motorDataR.MotoTemp       = (uint8_t)canRecvData.Data[6] - 40;
    			g_fbData.motorDataR.DriverTemp     = (uint8_t)canRecvData.Data[7] - 40;

    			break;
    		case MOTO_R_CANID3:
    			g_fbData.motorDataR.VoltL          = (uint8_t)canRecvData.Data[0];
    			g_fbData.motorDataR.VoltH          = (uint8_t)canRecvData.Data[1];
    			g_fbData.motorDataR.CurrentL       = (uint8_t)canRecvData.Data[2];
    			g_fbData.motorDataR.CurrentH       = (uint8_t)canRecvData.Data[3];
    			g_fbData.motorDataR.DistanceL      = (uint8_t)canRecvData.Data[4];
    			g_fbData.motorDataR.DistanceH      = (uint8_t)canRecvData.Data[5];
    			g_fbData.motorDataR.ErrCodeL       = (uint8_t)canRecvData.Data[6];
    			g_fbData.motorDataR.ErrCodeH       = (uint8_t)canRecvData.Data[7];

    			/* 收到心跳，重启定时器 */
    			Clock_setTimeout(clockMotoRearHeart,MOTO_CONNECT_TIMEOUT);
    			Clock_start(clockMotoRearHeart);
    			if(g_fbData.motorDataR.ErrCodeL == 0 && g_fbData.motorDataR.ErrCodeH == 0)
					rearValid = 1;
				else
					rearValid = 0;

    			break;
    		case MOTO_R_CANID4:
    			g_fbData.motorDataR.TorqueCtrlL    = (uint8_t)canRecvData.Data[0];
    			g_fbData.motorDataR.TorqueCtrlH    = (uint8_t)canRecvData.Data[1];
    			g_fbData.motorDataR.RPMCtrlL       = (uint8_t)canRecvData.Data[2];
    			g_fbData.motorDataR.RPMCtrlH       = (uint8_t)canRecvData.Data[3];
    			g_fbData.motorDataR.TorqueL        = (uint8_t)canRecvData.Data[4];
    			g_fbData.motorDataR.TorqueH        = (uint8_t)canRecvData.Data[5];
    			g_fbData.motorDataR.CanReserved1   = (uint8_t)canRecvData.Data[6];
    			g_fbData.motorDataR.CanReserved2   = (uint8_t)canRecvData.Data[7];
    			break;
    		default:
    			break;
		}/* switch */

		if((frontValid == 1 && canID == MOTO_F_CANID2) ||
		        (frontValid == 0 && rearValid == 1 && canID == MOTO_R_CANID2))
		{
		    if(frontValid == 1 && rearValid == 1)
		    {
		        recvRpm = ((g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML
		                   + (g_fbData.motorDataR.RPMH << 8) + g_fbData.motorDataR.RPML)/2;
		        maxThrottle = MAX_THROTTLE_SIZE/2;
		    }
		    else if(frontValid == 1)
		    {
		        recvRpm = (g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML;
		        maxThrottle = MAX_THROTTLE_SIZE;
		    }
		    else if(rearValid == 1)
		    {
		        recvRpm = (g_fbData.motorDataR.RPMH << 8) + g_fbData.motorDataR.RPML;
		        maxThrottle = MAX_THROTTLE_SIZE;
		    }
		    else;

		    g_fbData.recvRPM = recvRpm;
            g_fbData.calcRPM = calcRpm;


            /*
            if(g_fbData.motorDataF.RPML != 0 || g_fbData.motorDataF.RPMH != 0)
            {
                *(volatile uint16_t *)(SOC_EMIFA_CS2_ADDR + (0x5<<1)) = 0x5A;
                *(volatile uint16_t *)(SOC_EMIFA_CS2_ADDR + (0x5<<1)) = 0x00;
            }
            */

            if( MotoGetPidOn()  )
            {
                //recvRpm = (g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML;
                //recvThrottle = (g_fbData.motorDataF.ThrottleH << 8) + g_fbData.motorDataF.ThrottleL;

                if ( abs((int)calcRpm - (int)(MotoGetGoalRPM())) < DELTA_RPM ){
                    calcRpm = MotoGetGoalRPM();
                }
                else{
                    if(calcRpm<MotoGetGoalRPM() )
                    {
                        calcRpm+=DELTA_RPM;
                    }

                    else if(calcRpm>MotoGetGoalRPM() )
                    {
                        calcRpm-=DELTA_RPM;
                    }
                }

                calcRpm = calcRpm > RPM_LIMIT ? RPM_LIMIT : calcRpm;



                adjThrottle = MotoPidCalc(calcRpm,recvRpm,ParamInstance()->KP,
                                        ParamInstance()->KI,ParamInstance()->KU, 0);
#if 0
                if(hisThrottle < 0 && adjThrottle >0)
                {
                    hisThrottle += 5.0*adjThrottle;
                }
                else
                {
                    hisThrottle += adjThrottle;
                }
#endif
                hisThrottle += adjThrottle;

                if(hisThrottle > maxThrottle)
                    hisThrottle = maxThrottle;
                else if(hisThrottle < MIN_THROTTLE_SIZE)
                    hisThrottle = MIN_THROTTLE_SIZE;
                else;

                if(hisThrottle < 0)   //刹车状态
                {

                    MotoSetThrottle(0);

                    if(hisThrottle < BREAK_THRESHOLD)
                        adjbrake = - BRAKE_THRO_RATIO* (hisThrottle - BREAK_THRESHOLD);
                    else
                        adjbrake = 0;

                    if(adjbrake > MAX_BRAKE_SIZE)
                        BrakeSetBrake(MAX_BRAKE_SIZE);
                    else
                        BrakeSetBrake( round(adjbrake));


                }
                else  //油门
                {
                    MotoSetThrottle(round(hisThrottle));
                    BrakeSetBrake(0);
                }
            }
            else
            {
                calcRpm=0;
                hisThrottle = 0;
                MotoPidCalc(0,0,0,0,0,1);
            }

            Semaphore_post(pidSem);
		}/*if(canID == ....)*/

	}
}
//TODO: 发送机车状态到4G

void MotoSendFdbkToCellTask()
{
	/*
	 * 反馈数据包格式为 包头(0xAA 0x42 0x55) + 长度 + 数据 + 包尾(0x0D)
	 */
	uint8_t  sendbuff[256];
	int32_t tms;
	int bufflen = sizeof(g_fbData)+5;

	sendbuff[0] = 0xAA;
	sendbuff[1] = 0x42;
	sendbuff[2] = 0x55;
	sendbuff[3] = sizeof(g_fbData);
	sendbuff[bufflen-1] = 0x0D;

	while(1){
		/*
		fbData.motorDataR.ThrottleL++;
		fbData.motorDataR.RPMH++;
		fbData.motorDataF.ThrottleH++;
		fbData.motorDataF.RPML++;
		*/
		g_fbData.brake = BrakeGetBrake();
		g_fbData.railstate = RailGetRailState();
		userGetMS(&tms);
		g_fbData.rfidReadTime = tms;
		memcpy(sendbuff+4,(char*) &g_fbData, sizeof(g_fbData) );
		ZigbeeSend(sendbuff, bufflen);

		Task_sleep(100);
	}
}
void MototaskInit()
{
	Task_Handle task;
	Task_Params taskParams;

    /*初始化信用量*/
    MotoInitSem();

    /*初始化定时器*/
    MotoInitTimer();

    /*初始化CAN设备*/
    CanOpen(CAN_DEV_0, MotoCanIntrHandler, CAN_DEV_0);
    
    Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
    
	task = Task_create(MotoSendTask, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

    task = Task_create(MotoRecvTask, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}


	taskParams.priority = 2;
    task = Task_create(MotoSendFdbkToCellTask, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}

uint16_t MotoGetRealRPM(void)
{
	uint16_t uCarRPM;
	if (MotoGetMotoSel() == FRONT_ONLY)
	{
		uCarRPM = (g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML;
	}
	else if (MotoGetMotoSel() == REAR_ONLY)
	{
		uCarRPM = (g_fbData.motorDataR.RPMH << 8) + g_fbData.motorDataR.RPML;
	}
	else if (MotoGetMotoSel() == FRONT_REAR)
	{
		uCarRPM = ((g_fbData.motorDataF.RPMH << 8) + g_fbData.motorDataF.RPML + (g_fbData.motorDataR.RPMH << 8) + g_fbData.motorDataR.RPML) / 2;
	}

	return uCarRPM;
}
#if 0
float MotoPidCalc(int16_t expRpm,int16_t realRpm,float kp,float ki, uint8_t clear)
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

float MotoPidCalc(int16_t expRpm,int16_t realRpm,float kp,float ki, float ku,uint8_t clear)
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

uint8_t MotoSetErrorCode(uint8_t code)
{
    g_fbData.ErrorCode = code;
}

void MotoSetMotoSel(enum motoSel sel)
{
	m_motoCtrl.MotoSel = sel;
}
enum motoSel MotoGetMotoSel()
{
	return m_motoCtrl.MotoSel;
}
void MotoSetControlMode(enum motoMode mode)
{
	m_motoCtrl.ControlMode = mode;
}
enum motoMode MotoGetControlMode()
{
	return m_motoCtrl.ControlMode;
}
void MotoSetGear(enum motoGear gear)
{
	m_motoCtrl.Gear = gear;
}
enum motoGear MotoGetGear()
{
	return m_motoCtrl.Gear;
}
void MotoSetThrottle(uint8_t thr)
{
	m_motoCtrl.Throttle = thr;
}
uint8_t MotoGetThrottle()
{
	return m_motoCtrl.Throttle;
}
void MotoSetGoalRPM(uint16_t rpm)
{
	m_motoCtrl.GoalRPM = rpm;
}
uint16_t MotoGetGoalRPM()
{
	return m_motoCtrl.GoalRPM;
}

void MotoSetPidOn(uint8_t on)
{
	m_motoCtrl.PidOn = on;
}

uint8_t MotoGetPidOn()
{
	return m_motoCtrl.PidOn;
}


