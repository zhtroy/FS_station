/*
 * main.c
 *
 *  Created on: 2018年12月5日
 *      Author: zhtro
 */


#include "src-gen/Watch.h"
#include "stdio.h"
#include "Watch_fun.h"


/*
 * testStateMachine.c
 *
 *  Created on: 2018-12-2
 *      Author: zhtro
 */



#include "Message/Message.h"
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include "uartStdio.h"
#include "watch.h"
#include "uart.h"
#include "soc_C6748.h"


#include "testYAKINDU_HSM/src-gen/Watch.h"
#include "stdio.h"
#include "testYAKINDU_HSM/Watch_fun.h"



Void YAKINDUTask(UArg a0, UArg a1)
{
	p_msg_t msg;
	EventType et;

	Watch handle;
	watch_init(&handle);
	watch_enter(&handle);

	while(1){
		msg= Message_pend();

		//将msg (数据消息) 进行处理，映射到HSM的消息上去
		et = NONE;
		if(msg->type == uart){
			switch (msg->data[0]){
				case 's':
					et = SET;
					break;

				case 'm':
					et = MODE;
					break;


				default:
					et = NONE;

			}
		}
		else if (msg->type == timer)
		{
			et = TICK;
		}


		//hsm input

		watchIface_raise_ue(&handle, et);
	}
}

void UARTInputYAKINDUTask(UArg a0, UArg a1)
{
	uint8_t c;
	p_msg_t msg;
	while(1)
	{


		c = UARTCharGetNonBlocking(SOC_UART_1_REGS);

		msg = Message_getEmpty();

		msg ->type = uart;

		msg ->data[0] = c;

		Message_post(msg);

		Task_sleep(20);

	}
}





void TimerYAKINDUTask(UArg a0, UArg a1)
{
	p_msg_t msg;

	while(1)
	{
		msg = Message_getEmpty();
		msg->type = timer;
		Message_post(msg);
		Task_sleep(1000);
	}
}

void testYAKINDU_SM_init()
{
	Task_Handle task;
	Error_Block eb;
	Task_Params taskParams;


	Error_init(&eb);
    Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
	task = Task_create(YAKINDUTask, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

	//UART
	Task_Params_init(&taskParams);
	taskParams.priority = 3;
	taskParams.stackSize = 2048;
	taskParams.arg0 = 0;
	task = Task_create(UARTInputYAKINDUTask, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

	//timer
	Task_Params_init(&taskParams);
	taskParams.priority = 3;
	taskParams.stackSize = 2048;
	taskParams.arg0 = 0;
	task = Task_create(TimerYAKINDUTask, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}



}


