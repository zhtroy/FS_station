/*
 * test4GControl.c
 *
 *  用PC上的服务器发送指令，DSP上的4G模块接收后控制驱动电机，变轨电机，刹车电机
 *
 *  Created on: 2018-12-26
 *      Author: zhtro
 */


#include "Message/Message.h"
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include "stdio.h"
#include "stdlib.h"
#include "DSP_Uart/dsp_uart2.h"
#include "uartStdio.h"
#include <ti/sysbios/knl/Clock.h>
#include "Moto/task_ctrldata.h"
#include "task_moto.h"
#include "Test4GControl/test4GControl.h"
#include "Sensor/PhotoElectric/PhotoElectric.h"




/*
 * 全局变量，给Motor 和Brake 用
 */
extern ctrlData g_carCtrlData;
uint8_t g_connectStatus;
extern fbdata_t g_fbData;

static 	Clock_Handle _timer;


extern Void taskCellCommunication(UArg a0, UArg a1);
extern void testBrakeServoInit();
extern void testMototaskInit();
extern Void taskRFID(UArg a0, UArg a1);

static void FSprintf(const char *fmt, ...)
{
#if 0
	va_list vp;
    char func[128];
    char * str = func;
    va_start(vp,fmt);
    vsprintf(str,fmt,vp);
    UARTPuts(str,-1);
#endif
}

static xdc_Void connectionClosed(xdc_UArg arg)
{
	g_connectStatus = 0;
}

static void connectionOn()
{
	g_connectStatus = 1;
}

static void resetCarCtrlData()
{
	//初始化motor数据
		g_carCtrlData.MotoSel =2;  //两个电机都用
		g_carCtrlData.ControlMode = 0;  //Throttle模式
		g_carCtrlData.Gear = 0;    //空挡
		g_carCtrlData.Throttle = 0;
		g_carCtrlData.Rail = 0;
		g_carCtrlData.Brake = 0;
		g_carCtrlData.RPM = 0;

		FSprintf("reset motor\n");
}

static void setCarRPM(uint16_t value)
{
	g_carCtrlData.RPM = value;
	FSprintf("set RPM %d\n", value);
}
static void setCarBrake(uint8_t value)
{
	g_carCtrlData.Brake = value;
	FSprintf("set brake %d\n", value);
}
static void setCarThrottle(uint8_t value)
{
	g_carCtrlData.Throttle = value;
	FSprintf("set throttle %d\n", value);
}

static void sendTimeMsg(UArg arg)
{
	p_msg_t msg;

	msg = Message_getEmpty();
	msg->type = timer;
	msg->data[0] = (uint8_t) arg;

	Message_post(msg);
}

static void InitTimer()
{
	Clock_Params clockParams;

	Error_Block  eb;
	Error_init(&eb);

	Clock_Params_init(&clockParams);
	clockParams.period = 0;       // one shot
	clockParams.startFlag = FALSE;

	_timer = Clock_create(0, 1000, &clockParams, &eb);
}



static void setTimeout(uint32_t timeout,uint8_t msg)
{
	Clock_stop(_timer);
	Clock_setFunc(_timer, sendTimeMsg, (UArg) msg);
	Clock_setTimeout(_timer, timeout);
	Clock_start(_timer);
}


static Void task4GControlMain(UArg a0, UArg a1)
{
	p_msg_t msg;
	int tempint, paramNum;
	car_mode_t carMode = Manual; //手动0 设置参数1 自动2
	car_state_t carState = idle;
	car_seperate_state_t seperateState;
	car_merge_state_t mergeState;


    uint16_t RPMparam[car_state_None] = {0};

    InitTimer();

	 // 使用一个Timer来检测4G通信心跳包
	Clock_Params clockParams;
	Clock_Handle heartClock;
	Error_Block  eb;

	Error_init(&eb);

	Clock_Params_init(&clockParams);
	clockParams.period = 0;       // one shot
	clockParams.startFlag = FALSE;
	heartClock = Clock_create(connectionClosed, 500, &clockParams, &eb); //500ms 后没有收到包就停止
	if ( heartClock == NULL )
	{
		System_abort("Clock create failed\n");
	}

	// Timer 配置结束


	while(1){
		msg= Message_pend();


		if(msg->type == cell)
		{
			/*
			 * 检测心跳包
			 */
			//连接状态 on
			connectionOn();
			//清零clock
			Clock_start(heartClock);

			/*
			 * 模式跳转
			 * 0 手动模式
			 * 1 设置模式
			 * 2 自动模式
			 * 3 紧急制动模式
			 */
			if(msg->data[0] == 'z'){
				tempint = atoi(&(msg->data[1]));   //转换字符串为int
				carMode =(car_mode_t) tempint; //模式跳转
				resetCarCtrlData();  //清零电机控制
				g_carCtrlData.AutoMode = carMode;

				switch(carMode)   ///模式跳转动作
				{
					case Auto:
						g_carCtrlData.Gear = 1;   //进入自动模式后，默认挂前进档
						carState = idle;
						break;

				}
			}
		}

		g_fbData.mode = (uint8_t) carMode;
		g_fbData.FSMstate = carState;

		if(carMode == Manual){ //手动模式
			switch(msg->type){
				case cell:
				{
					/*
					 *  TODO：简单的命令格式，后续需要改为和RFID协议类似的
					 *   第一位字符表示命令类型，后面的字符串转成uint8_t作为数据
					 *
					 */
					tempint = atoi(&(msg->data[1]));   //转换字符串为int

					switch(msg->data[0])
					{
						case 'm':    //motosel
							g_carCtrlData.MotoSel = (uint8_t) tempint;
							break;
						case 'c':    //controlmode
							g_carCtrlData.ControlMode = (uint8_t) tempint;
							break;
						case 'g':    //gear
							g_carCtrlData.Gear = (uint8_t) tempint;
							break;
						case 't':    //throttle
							g_carCtrlData.Throttle = (uint8_t) tempint;
							break;
						case 'r':    //rail
							ChangeRailStart();
							break;
						case 'R': //railstate
							setRailState((uint8_t) tempint);
							break;
						case 'b':    //brake
							g_carCtrlData.Brake = (uint8_t) tempint;
							break;
						case 'h' :   //心跳包
							break;
					}

					break;
				}   //case cell

			}
		}////手动模式

		else if(carMode == Setting)  //设置模式
		{
			switch(msg->type){
				case cell:
				{
					tempint = atoi(&(msg->data[1]));   //转换字符串为int
					switch(msg->data[0])
					{
						case 's':
							//设置RPM参数  4位
							paramNum = (msg->data[1]-'0')*10 + (msg->data[2]-'0');

							RPMparam[paramNum] = (msg->data[3]-'0')*1000 + (msg->data[4]-'0')*100 +(msg->data[5]-'0')*10 + (msg->data[6]-'0');
							break;

						case 'k': //设置PID参数  1000000倍
							switch(msg->data[1]){
							case 'i':
								g_carCtrlData.KI = (msg->data[2]-'0')*1000000 + (msg->data[3]-'0')*100000\
												+(msg->data[4]-'0')*10000 +(msg->data[5]-'0')*1000+(msg->data[6]-'0')*100\
												+(msg->data[7]-'0')*10 + (msg->data[8]-'0');
								break;
							case 'p':
								g_carCtrlData.KP = (msg->data[2]-'0')*1000000 + (msg->data[3]-'0')*100000\
												+(msg->data[4]-'0')*10000 +(msg->data[5]-'0')*1000+(msg->data[6]-'0')*100\
												+(msg->data[7]-'0')*10 + (msg->data[8]-'0');
								break;
							case 'u':
								g_carCtrlData.KU = (msg->data[2]-'0')*1000000 + (msg->data[3]-'0')*100000\
												+(msg->data[4]-'0')*10000 +(msg->data[5]-'0')*1000+(msg->data[6]-'0')*100\
												+(msg->data[7]-'0')*10 + (msg->data[8]-'0');
								break;

							}
							break;


					}
					break;
				}  //case cell
			}
		}

		else if (carMode == Auto) //进入自动模式
		{

			switch(carState)
			{
				case idle:
				{
					if(msg->type == cell){
						if(msg->data[0] == 'S' )   //开始巡航
						{
							carState = cruising;
							setCarRPM(RPMparam[carState]);
						}
					}
					break;
				}
				case cruising:
				case straight:
				case pre_curve:
				case curving:
				case uphill:
				case pre_downhill:
				case downhill:
				{
					switch(msg->type)
					{
						case rfid:
						{
							switch(msg->data[0])
							{
								case EPC_STRAIGHT:
									carState = straight;

									break;
								case EPC_PRE_CURVE:
									carState = pre_curve;

									break;
								case EPC_CURVING:
									carState = curving;

									break;
								case EPC_UPHILL:
									carState = uphill;

									break;
								case EPC_PRE_DOWNHILL:
									carState = pre_downhill;

									break;
								case EPC_DOWNHILL:
									carState = downhill;

									break;
								case EPC_PRE_SEPERATE:
									carState = pre_seperate;

									break;

							}
							setCarRPM(RPMparam[carState]);
							break;
						}
					}

					break;
				} //case downhill

				case pre_seperate:
				{
					if(msg->type == rfid && msg->data[0] == EPC_SEPERATE)
					{
						carState = seperate;
						//action
						setCarRPM(RPMparam[carState]);
						setTimeout(g_timeout[seperate_wait_photon],seperate_wait_photon);
						seperateState = s_wait_photon;
					}
					break;
				} //case pre_seperate

				case seperate:
				{
					switch(seperateState)
					{
						case s_wait_photon:
						{
							if(msg->type == timer && msg->data[0] == seperate_wait_photon)
							{
								seperateState = seperate_cancel;
								FSprintf("seperate_wait_photon timeout \n");
							}

							if(msg->type == photon)
							{
								seperateState = seperating;
								//action
								setTimeout(g_timeout[seperate_wait_changerail], seperate_wait_changerail);
								ChangeRailStart();
								FSprintf("begin change rail \n");
							}
							break;

						}

						case seperating:
						{
							if(msg->type == timer && msg->data[0] == seperate_wait_changerail)
							{
								FSprintf("seperate_wait_changerail timeout\n");
								if(ChangeRailIsComplete())
								{
									seperateState = seperate_ok;
									FSprintf("seperate OK\n");
								}
								else{
									//TODO: 紧急制动
									carMode = ForceBrake;
									FSprintf("seperate failed\n");
								}
							}
							break;
						}

						case seperate_ok:
						{
							//FIXME: 特殊处理
							if(msg->type == rfid && msg->data[0] ==EPC_ENTER_STATION)
							{
								carState = enter_station;
								setCarRPM(RPMparam[carState]);

							}
							break;
						}

						case seperate_cancel:
						{
							//FIXME: 特殊处理
							if(msg->type == rfid && msg->data[0] ==EPC_STRAIGHT)
							{
								carState = straight;
								setCarRPM(RPMparam[carState]);
							}
							break;
						}
					}

					break;
				}//case seperate:

				case enter_station:
				{
					if(msg->type == rfid && msg->data[0] == EPC_STOP_STATION){
						carState = stop_station;
						setCarRPM(0);
						setTimeout(g_timeout[station_stop], station_stop);
					}
					break;
				} //case enter_station:

				case stop_station:
				{
					if(msg->type==timer && msg->data[0] == station_stop){
						setCarRPM(RPMparam[carState]);
						FSprintf("station_stop timeout\n");
					}
					if(msg->type == rfid && msg->data[0] == EPC_LEAVE_STATION)
					{
						carState = leave_station;
						setCarRPM(RPMparam[carState]);
					}
					break;
				}

				case leave_station:
				{
					if(msg->type == rfid && msg->data[0] == EPC_PRE_MERGE)
					{
						carState = pre_merge;
						setCarRPM(RPMparam[carState]);
					}
					break;
				}

				case  pre_merge:
				{
					if(msg->type == rfid && msg->data[0] == EPC_MERGE)
					{
						carState = merge;
						mergeState = m_wait_photon;
						setCarRPM(RPMparam[carState]);
						setTimeout(g_timeout[merge_wait_photon],merge_wait_photon);
					}
					break;
				}

				case merge:
				{
					switch(mergeState)
					{
						case m_wait_photon:
						{
							if(msg->type==timer && msg->data[0] == merge_wait_photon){
								//TODO: 制动
								carMode = ForceBrake;
								FSprintf("merge_wait_photon timeout\n");
							}
							if(msg->type == photon)
							{
								mergeState = merging;
								//action
								setTimeout(g_timeout[merge_wait_changerail], merge_wait_changerail);
								ChangeRailStart();
							}

							break;
						}

						case merging:
						{
							if(msg->type==timer && msg->data[0] == merge_wait_changerail){
								FSprintf("merge_wait_changerail timeout\n");
								if(ChangeRailIsComplete()){
									mergeState = merge_ok;
									FSprintf("merge ok\n");
								}
								else{
									//紧急刹车
									carMode = ForceBrake;
									FSprintf("merge failed\n");
								}
							}
							break;
						}

						case merge_ok:
						{
							//FIXME: 特殊处理
							if(msg->type == rfid && msg->data[0] == EPC_PRE_CURVE){
								carState = pre_curve;
								setCarRPM(RPMparam[carState]);
							}
							break;
						}
					}
				} //case merge:

			}


		}  //else if(autoMode == 2)  //自动

		if(carMode == ForceBrake)
		{
			g_carCtrlData.AutoMode = carMode;
			setCarBrake(FORCE_BRAKE);
			setCarThrottle(0);
		}

		Message_recycle(msg);
	}
}



void test4GControl_init()
{
	Task_Handle task;
	Error_Block eb;
	Task_Params taskParams;


	Error_init(&eb);
    Task_Params_init(&taskParams);
	taskParams.priority = 6;      //比5高
	taskParams.stackSize = 2048;
	task = Task_create(task4GControlMain, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

	//4G
	Task_Params_init(&taskParams);
	taskParams.priority = 3;
	taskParams.stackSize = 2048;
	taskParams.arg0 = 0;
	task = Task_create(taskCellCommunication, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}


	//驱动电机  priority = 5
	testMototaskInit();
	//刹车       priority = 5
	testBrakeServoInit();

	//变轨电机

	//RFID

	Task_Params_init(&taskParams);
	taskParams.priority = 3;
	taskParams.stackSize = 2048;
	taskParams.arg0 = 0;
	task = Task_create(taskRFID, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

	//对管
	Task_Params_init(&taskParams);
	taskParams.priority = 3;
	taskParams.stackSize = 2048;
	taskParams.arg0 = 0;
	task = Task_create(taskPhotoElectric, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}
