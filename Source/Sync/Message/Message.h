/*
 * messageQueue.h
 *
 *  Created on: 2018-12-2
 *      Author: zhtro
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <ti/sysbios/knl/Queue.h>

#define NUMMSGS 64 /* number of messages */
#define MSGSIZE (128)

/*消息优先级宏定义*/
#define MSG_PRI_LOW (0)
#define MSG_PRI_MID (128)
#define MSG_PRI_HIGH (256)

typedef enum{
	rfid,
	mmradar,
	uart,
	timer,
	sonicradar,
	cell,               //4G数据
	photon,				//对管
	changerail, 		//变轨
	motor,				//动力电机
	brake,    			//刹车
	error,				//错误
	zigbee,             //Zigbee
	Empty               //缺省类型
}msg_type_t;

typedef struct MsgObj {
	Queue_Elem elem; /* first field for Queue */
	uint8_t	   pri;  /* 消息优先级，共256级*/
	msg_type_t type; /*msg type */
	uint8_t data[MSGSIZE]; /* message value */
	uint32_t dataLen;      //数据长度
} msg_t, *p_msg_t;


//API=========================
void Message_init();
/*
 * 创建一个消息
 */
p_msg_t Message_new(uint8_t	   pri,  /* 消息优先级，共256级*/
					msg_type_t type,  /*msg type */
					uint8_t data[], /* message value */
					uint32_t dataLen      //数据长度
					);

p_msg_t Message_getEmpty();
void Message_recycle(p_msg_t msg);
p_msg_t Message_pend();
void Message_post_pri(p_msg_t msg, uint8_t pri);
void Message_post(p_msg_t msg);
char* Message_getNameByType(msg_type_t t);


#endif /* MESSAGE_H_ */
