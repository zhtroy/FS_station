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



#define CELL_BUFF_SIZE (128)
#define LTE_WORK_MODE (1)
#define LTE_TEST_MODE (0)

static Semaphore_Handle sem_cell_data_received;
static int8_t cell_data_buffer[CELL_BUFF_SIZE];
static uint16_t buff_head = 0;
static uint16_t buff_tail = 0;
extern int8_t lteMode;


typedef enum{
	cell_wait,
	cell_recv
}cell_state_t;

#define CELL_HEAD ('$')
#define CELL_END  ('^')

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
        		}
        		break;

        	case cell_recv:
        		if(c == CELL_END)
        		{
        			state = cell_wait;
        			//发送命令给主任务
        			pmsg = Message_getEmpty();
        			pmsg->type = cell;

        			memset(pmsg->data,0,MSGSIZE);  //清零data
        			memcpy(pmsg->data, command, commandLen);

        			pmsg->dataLen = commandLen;

        			Message_post(pmsg);
        		}
        		else      //接收命令字符
        		{
        			command[commandLen++] = c;
        		}
        		break;
        	}

        }
        else    /*lteMode.workMode = LTE_TEST_MODE*/
        {
            vOutputString(&c,1);
        }
        buff_head = (buff_head+1) % CELL_BUFF_SIZE;

	}
}
