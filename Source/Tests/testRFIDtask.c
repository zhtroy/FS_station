/*
 * testRFIDtask.c
 *
 *  Created on: 2018-12-4
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


extern Void taskRFID(UArg a0, UArg a1);

Void taskTestRFIDMain(UArg a0, UArg a1)
{
	p_msg_t msg;

	while(1){
		msg= Message_pend();
		if(msg->type==rfid){
			System_printf("msg type = %s\t epc= %x\n", Message_getNameByType(msg->type), msg->data[0]);
		}

	}
}

void testRFIDtask()
{
	Task_Handle task;
	Error_Block eb;
	Task_Params taskParams;


	Error_init(&eb);
    Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
	task = Task_create(taskTestRFIDMain, &taskParams, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

	//UART
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
