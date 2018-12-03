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

typedef enum{
	rfid,
	mmradar,
	uart,
	timer,
	Empty               //缺省类型
}msg_type_t;

typedef struct MsgObj {
	Queue_Elem elem; /* first field for Queue */
	msg_type_t type; /*msg type */
	uint8_t data[MSGSIZE]; /* message value */
} msg_t, *p_msg_t;


//API=========================
void Message_init();
p_msg_t Message_getEmpty();
p_msg_t Message_pend();
void Message_post(p_msg_t);
char* Message_getNameByType(msg_type_t t);


#endif /* MESSAGE_H_ */
