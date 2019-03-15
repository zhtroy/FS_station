/*
 * Message.c
 *
 * 提供一个带优先级的消息队列，使用信号量和SYS/BIOS自带的Queue实现
 *
 * 使用方式:
 *     发布者:	1.msg = Message_new() 获取一个空的消息,并填充内容
 *     		 	2.Message_post(msg)	   发送消息到队列中
 *
 *     接收者:	1.msg = Message_pend()    阻塞等待一个消息
 *     		 	2.Message_recycle(msg)	  回收消息内存到freeQueue中
 *
 *  Created on: 2018-12-1
 *      Author: zhtro
 */
//TODO:
#include <xdc/std.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/hal/Hwi.h>
#include <xdc/runtime/System.h>

#include "stdint.h"
#include "Message/Message.h"


static Semaphore_Handle sem_msg;
static Queue_Handle msgQueue;
static Queue_Handle freeQueue;

static char * typeToName[]= {
		"rfid",
		"mmradar",
		"uart",
		"timer",
		"sonicradar",
		"cell",
		"Empty"
};


void Message_init(){


	Int i;
	p_msg_t msg;
	Error_Block eb;

    // 创建一个信号量
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_COUNTING;
    sem_msg = Semaphore_create(0, &semParams, NULL);

    msgQueue = Queue_create(NULL,NULL);
    freeQueue = Queue_create(NULL,NULL);


    //分配若干条消息的内存
	Error_init(&eb);
	msg = (msg_t *) Memory_alloc(NULL, NUMMSGS * sizeof(msg_t), 0, &eb);
	if (msg == NULL) {
		System_abort("Memory allocation failed");
	}


	/* Put all messages on freeQueue */
	for (i = 0; i < NUMMSGS; msg++, i++) {
		Queue_put(freeQueue, (Queue_Elem *) msg);
	}


}

//从freeQueue中获取一条空的消息
p_msg_t Message_getEmpty()
{
	p_msg_t msg;

	msg = Queue_get(freeQueue);
	/* fill in value */
	msg->type = Empty;

	return msg;
}

/*
 * 创建一个消息
 */
p_msg_t Message_new(uint8_t	   pri,  /* 消息优先级，共256级*/
					msg_type_t type,  /*msg type */
					uint8_t data[], /* message value */
					uint32_t dataLen      //数据长度
					)
{
	p_msg_t msg;

	msg = Message_getEmpty();
	memset(msg,0, sizeof(msg_t));
	msg->pri = pri;
	msg->type = type;
	msg->dataLen = dataLen;
	memcpy(msg->data, data, dataLen);

	return msg;
}
void Message_recycle(p_msg_t msg)
{
	/* put message */
	Queue_put(freeQueue, (Queue_Elem *) msg);
}

//等待一条消息
p_msg_t Message_pend()
{
	p_msg_t msg;

	Semaphore_pend(sem_msg, BIOS_WAIT_FOREVER);
	/* get message */
	msg = Queue_get(msgQueue);

	return msg;
}


/*发送一条消息*/
void Message_post_pri(p_msg_t msg, uint8_t pri)
{
	UInt key;
	p_msg_t pInsert;

	/* 将消息按照优先级升序的方式插入到队列中
	 *
	 * 关闭中断，保证原子操作
	 */
	key = Hwi_disable();

	pInsert = (p_msg_t) Queue_head(msgQueue);
	while(pInsert->pri >= pri &&
			&(pInsert->elem) != (Queue_Elem *)msgQueue )
	{
		pInsert= (p_msg_t) Queue_next(&(pInsert->elem));
	}

	Queue_insert(&(pInsert->elem), &(msg->elem));

	Hwi_restore(key);

	/* post semaphore */
	Semaphore_post(sem_msg);
}

/*
 *无优先级默认为 MSG_PRI_MID
 */
void Message_post(p_msg_t msg)
{
	Message_post_pri(msg, MSG_PRI_MID);
}

//输入 msg_type_t 返回消息类型字符串
char* Message_getNameByType(msg_type_t t)
{
	return typeToName[(int) t];
}
