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




/*
 * 全局变量，给Motor 和Brake 用
 */
extern ctrlData carCtrlData;
uint8_t connectStatus;
extern fbdata_t fbData;


/*
 * 状态机
 */
typedef enum{
	stationary,
	straight,
	turning,
	uphill,
	downhill
}car_state_t;

typedef struct{
	uint8_t throttle;
	uint8_t brake;
}auto_param_t;

extern Void taskCellCommunication(UArg a0, UArg a1);
extern void testBrakeServoInit();
extern void testMototaskInit();
extern Void taskRFID(UArg a0, UArg a1);


static xdc_Void connectionClosed(xdc_UArg arg)
{
	connectStatus = 0;
}

static void connectionOn()
{
	connectStatus = 1;
}

static void resetCarCtrlData()
{
	//初始化motor数据
		carCtrlData.MotoSel =2;  //两个电机都用
		carCtrlData.ControlMode = 0;  //Throttle模式
		carCtrlData.Gear = 0;    //空挡
		carCtrlData.Throttle = 0;
		carCtrlData.Rail = 0;
		carCtrlData.Brake = 0;

		UARTprintf("reset motor\n");
}

static void setCarBrake(uint8_t value)
{
	carCtrlData.Brake = value;
	UARTprintf("set brake %d\n", value);
}
static void setCarThrottle(uint8_t value)
{
	carCtrlData.Throttle = value;
	UARTprintf("set throttle %d\n", value);
}
static Void task4GControlMain(UArg a0, UArg a1)
{
	p_msg_t msg;
	int value, paramNum;
	int autoMode = 0; //手动0 设置参数1 自动2
//	car_state_t carState = stationary;

    auto_param_t params[7];
    memset(params,0,sizeof(params));

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
		UARTprintf("mode: %d\n",autoMode);
		fbData.mode = autoMode;
		if(autoMode == 0){ //手动模式
			switch(msg->type){
				case cell:
				{
					//连接状态 on
					connectionOn();
					//清零clock
					Clock_start(heartClock);

					/*
					 *  TODO：简单的命令格式，后续需要改为和RFID协议类似的
					 *   第一位字符表示命令类型，后面的字符串转成uint8_t作为数据
					 *
					 */
					value = atoi(&(msg->data[1]));   //转换字符串为int

					switch(msg->data[0])
					{
						case 'm':    //motosel
							carCtrlData.MotoSel = (uint8_t) value;
							break;
						case 'c':    //controlmode
							carCtrlData.ControlMode = (uint8_t) value;
							break;
						case 'g':    //gear
							carCtrlData.Gear = (uint8_t) value;
							break;
						case 't':    //throttle
							carCtrlData.Throttle = (uint8_t) value;
							break;
						case 'r':    //rail
							carCtrlData.Rail = (uint8_t) value;
							break;
						case 'b':    //brake
							carCtrlData.Brake = (uint8_t) value;
							break;
						case 'h' :   //心跳包
							break;
						case 'z':    //切换到设置模式
							autoMode = value; //状态跳转
							resetCarCtrlData();  //清零电机控制

							break;

					}

					break;
				}   //case cell

			}
		}////手动模式
		else if(autoMode == 1)  //设置模式
		{
			switch(msg->type){
				case cell:
				{


					value = atoi(&(msg->data[1]));   //转换字符串为int
					switch(msg->data[0])
					{
						case 'z':    //切换模式

							autoMode = value; //状态跳转
							resetCarCtrlData();  //清零电机控制
							if(value == 2){
								carCtrlData.Gear = 1;
							}

							break;
						case 's':    //设置油门，刹车参数
							paramNum = (msg->data[1]-'0')*10 + (msg->data[2]-'0');
							params[paramNum].throttle = (msg->data[3]-'0')*100 + (msg->data[4]-'0')*10 + (msg->data[5]-'0');
							params[paramNum].brake = (msg->data[6]-'0')*100 + (msg->data[7]-'0')*10 + (msg->data[8]-'0');
							break;

					}
					break;
				}

			}
		}
		else if (autoMode == 2) //进入自动模式
		{
			switch(msg->type){
				case cell:
				{
					//连接状态 on
					connectionOn();
					//清零clock
					Clock_start(heartClock);

					value = atoi(&(msg->data[1]));   //转换字符串为int
					switch(msg->data[0])
					{
						case 'z':    //切换模式
							autoMode = value; //状态跳转
							resetCarCtrlData();  //清零电机控制
							break;
					}
					break;
				}

				case rfid:
				{
					/*
					epc = getEPC(msg->data);
					switch(carState){
						case stationary:
						{
							switch(epc.epcType)
							{
								case
							}
						}
					}
					*/
					value = msg->data[0];
					if(value<=7 && value>=1){
						setCarBrake(params[value-1].brake);
						setCarThrottle(params[value-1].throttle);
					}
					else
					{
						setCarBrake(0);
						setCarThrottle(0);
					}
					/*
					switch(value){  //无状态
						case 1:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						case 2:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						case 3:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						case 4:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						case 5:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						case 6:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						case 7:
							setCarBrake(0);
							setCarThrottle(20);
							break;
						default:
							setCarBrake(0);
							setCarThrottle(0);
					}
					*/

					break;
				}
			}
		}  //else if(autoMode == 2)  //设置自动

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
}
