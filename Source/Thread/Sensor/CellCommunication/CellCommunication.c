/*
 * CellCommunication.c
 *  4G 模块
 *
 *  Created on: 2018-12-8
 *      Author: zhtro
 */

#include <xdc/std.h>
#include "DSP_Uart/dsp_uart2.h"
#include "soc_C6748.h"
#include "uart.h"
#include "stdio.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include "stdint.h"
#include "interrupt.h"
#include <xdc/runtime/System.h>
#include "Message/Message.h"
#include "Sensor/CellCommunication/CellCommunication.h"
#include "Sensor/CellCommunication/NetPacket.h"





static Semaphore_Handle sem_cell_data_received;
static int8_t cell_data_buffer[CELL_BUFF_SIZE];
static uint16_t buff_head = 0;
static uint16_t buff_tail = 0;
extern int8_t lteMode;

char cell_packet_divider[] = "SOCKA:";


extern void vOutputString( const char * const pcMessage,int numBytesToWrite);

/****************************************************************************/
/*                                                                          */
/*              硬件中断线程                                                */
/*                                                                          */
/****************************************************************************/

//在cfg 文件中静态配置
void DSPUART2Isr(void)
{
    unsigned char rxData = 0;
    unsigned int int_id = 0;
	// 使能中断
	unsigned int intFlags = 0;
    intFlags |= (UART_INT_LINE_STAT  |  \
                 UART_INT_RXDATA_CTI);

    // 确定中断源
    int_id = UARTIntStatus(SOC_UART_2_REGS);
    UARTIntDisable(SOC_UART_2_REGS, intFlags);
    IntEventClear(SYS_INT_UART2_INT);



    // 接收中断
    if(UART_INTID_RX_DATA == int_id)
    {
        cell_data_buffer[buff_tail] = UARTCharGetNonBlocking(SOC_UART_2_REGS);
        buff_tail = (buff_tail+1) % CELL_BUFF_SIZE;
        Semaphore_post(sem_cell_data_received);
//        rxData = UARTCharGetNonBlocking(SOC_UART_2_REGS);
//        UARTCharPutNonBlocking(SOC_UART_2_REGS, rxData);
    }

    // 接收错误
    if(UART_INTID_RX_LINE_STAT == int_id)
    {
        while(UARTRxErrorGet(SOC_UART_2_REGS))
        {
            // 从 RBR 读一个字节
            UARTCharGetNonBlocking(SOC_UART_2_REGS);
        }
    }

    UARTIntEnable(SOC_UART_2_REGS, intFlags);
}


static void SemInit()
{
	Semaphore_Params semParams;

	//创建信号量
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_COUNTING;
	sem_cell_data_received = Semaphore_create(0, &semParams, NULL);
}

/****************************************************************************/
/*                                                                          */
/*              函数定义                                                        */
/*                                                                          */
/****************************************************************************/

void CellSendData(char * pbuff, uint32_t size)
{
	//使用阻塞发送
	UART2Send(pbuff, size);

}

/*
 * 发送心跳包线程
 */
static void taskSendHeartBeatToServer()
{
	net_packet_t packet;
	int len;

	NetPacketCtor(&packet,0x01,0x1001,0x0001,0x42,2,0,0);
	len = NetPacketToNetOrder(&packet);

	while(1)
	{
		Task_sleep(1000);
		CellSendData((char*) &packet, len);
	}
}
/*
 * 通过4G模块与上海后台服务器通信
 */
Void taskServerCommunication(UArg a0, UArg a1)
{
	Task_Handle task;
	Task_Params taskParams;

	int8_t c;
	cell_state_new_t state = CELL_DIV;
	net_packet_t packet;
	int div_num = 0;
	int recv_head_num = 0;
	char headbuff[HDR_LEN];
	int recv_data_num = 0;
	p_msg_t pmsg;

	SemInit();


	Task_Params_init(&taskParams);
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
	task = Task_create(taskSendHeartBeatToServer, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}

	while(1)
	{
		Semaphore_pend(sem_cell_data_received, BIOS_WAIT_FOREVER);
		c = cell_data_buffer[buff_head];
		switch(state)
		{
		case CELL_DIV:
			if(c == cell_packet_divider[div_num]){
				div_num ++;
				if(div_num>=sizeof(cell_packet_divider)-1)
				{
					div_num = 0;
					state = CELL_RECV_HEAD;
				}
			}
			else{
				div_num = 0;
			}

			break;

		case CELL_RECV_HEAD:
			headbuff[recv_head_num] = c;
			recv_head_num++;
			if(recv_head_num>=HDR_LEN)
			{
				NetPacketBuildHeaderFromRaw(&packet,headbuff);
				recv_head_num=0;
				state = CELL_RECV_DATA;
			}
			break;

		case CELL_RECV_DATA:
			packet.data[recv_data_num] = c;
			recv_data_num++;
			if(recv_data_num >= packet.len-HDR_LEN)
			{
				recv_data_num = 0;
				state = CELL_DIV;
				//TODO: 接收到一个完整包，进行后续处理
				pmsg = Message_getEmpty();

				pmsg->type = cell;

				*((uint16_t*) (pmsg->data)) = packet.cmd;

				Message_post(pmsg);

			}

		} //switch

		buff_head = (buff_head+1) % CELL_BUFF_SIZE;
	}
}
/*
 * TODO: 将遥控功能从4G模块转移到其他的无线模块上
 * 使用4G模块实现遥控功能
 * 接收4G数据， 将数据按 $command^分包后放入消息队列
 */
Void taskCellCommunication(UArg a0, UArg a1)
{
	uint8_t txbuff[] = {"hello 4G, this is a test"};
	uint8_t rxbuff[100];

	int8_t c;
	int8_t command[100] = {0};
	int commandLen = 0;
	p_msg_t pmsg;

	cell_state_t state = cell_wait;


	SemInit();


	while(1)
	{
		Semaphore_pend(sem_cell_data_received, BIOS_WAIT_FOREVER);
        c = cell_data_buffer[buff_head];
        if(lteMode == LTE_WORK_MODE)
        {
        //		System_printf("%c\n", cell_data_buffer[buff_head]);

        	//解析4G数据
        	/*
        	 * 简单的包头包尾
        	 * 命令格式为 $command^
        	 */
        	switch(state)
        	{
        	case cell_wait:
        		if (c == CELL_HEAD)
        		{
        			state = cell_recv;
        			commandLen = 0;
        			pmsg = Message_getEmpty();
        			memset(pmsg->data,0,MSGSIZE);  //清零data
        		}
        		break;

        	case cell_recv:
        		if(c == CELL_END)
        		{
        			state = cell_wait;
        			//发送命令给主任务

        			pmsg->type = cell;

        			pmsg->dataLen = commandLen;

        			Message_post(pmsg);
        		}
        		else      //接收命令字符
        		{
        			pmsg->data[commandLen++] = c;
        		}
        		break;
        	}

        }
        else    /*lteMode.workMode == LTE_TEST_MODE*/
        {
            vOutputString(&c,1);
        }
        buff_head = (buff_head+1) % CELL_BUFF_SIZE;

	}
}
