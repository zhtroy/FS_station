/*
 * testZCP.c
 *
 *  Created on: 2019-5-5
 *      Author: DELL
 */
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include <xdc/std.h>
#include "ZCP/zcp_driver.h"

#define CAR_ZCP_UART_DEV_NUM    (5)
#define CAR_ZCP_DEV_NUM (0)

ZCPInstance_t carInst;
Semaphore_Handle carSem;

void testZCPSendTask(UArg arg0, UArg arg1)
{
    ZCPUserPacket_t sendPacket;

    sendPacket.addrH = 0x80;
    sendPacket.addrL = 0x01;
    sendPacket.len = 0x06;
    sendPacket.type = 0x40;
    sendPacket.data[0] = 0x01;
    sendPacket.data[1] = 0x02;
    sendPacket.data[2] = 0x03;
    sendPacket.data[3] = 0x04;
    sendPacket.extSec = 0x01;

    while(1)
    {
        Task_sleep(100);
        if(FALSE == Mailbox_post(carInst.userSendMbox, (Ptr *)&sendPacket, BIOS_NO_WAIT))
        {
            LogMsg("Send Failed!\r\n");
        }

        Semaphore_pend(carSem,BIOS_WAIT_FOREVER);
    }
}


void testZCPRecvTask(UArg arg0, UArg arg1)
{
    ZCPUserPacket_t recvPacket;
    while(1)
    {
        Mailbox_pend(carInst.userRecvMbox, (Ptr *)&recvPacket, BIOS_WAIT_FOREVER);
        if(recvPacket.type == ZCP_TYPE_ERROR || recvPacket.type == ZCP_TYPE_ACK)
        {
            if(recvPacket.extSec == 0x01)
            {
                Semaphore_post(carSem);
            }
        }
        else
        {
            LogMsg("Addr:%x%x,len:%d,type:%d\r\n",
                    recvPacket.addrH,
                    recvPacket.addrL,
                    recvPacket.len,
                    recvPacket.type);
        }
    }
}

void testZCPInit()
{
    Semaphore_Params semParams;
    Task_Handle task;
    Task_Params taskParams;

    ZCPInit(&carInst,CAR_ZCP_DEV_NUM,CAR_ZCP_UART_DEV_NUM);

    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    carSem = Semaphore_create(0, &semParams, NULL);

    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;

    task = Task_create((Task_FuncPtr)testZCPSendTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    task = Task_create((Task_FuncPtr)testZCPRecvTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }
}
