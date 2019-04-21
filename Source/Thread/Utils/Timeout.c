/*
 * Timeout.c
 *
 *  Created on: 2019-4-19
 *      Author: zhtro
 */


#include "Message/Message.h"
#include <ti/sysbios/knl/Clock.h>
#include "stdint.h"
#include "Utils/Timeout.h"

static 	Clock_Handle _timer;

static uint32_t m_timeout[]=
{
//		seperate_wait_photon
		2000,
//		seperate_wait_changerail
		3000,
//		station_stop
		5000,
//		merge_wait_photon
		2000,
//		merge_wait_changerail
		4000,
//		seperate_wait_enter_station
		20000,
//		seperate_wait_stop_station
		10000,
//		seperate_wait_leave_station
		10000,
//		seperate_wait_pre_merge
		10000,
//		seperate_wait_merge
		10000
};
static void sendTimeMsg(UArg arg)
{

	p_msg_t msg;

	msg = Message_getEmpty();
	msg->type = timer;
	msg->data[0] = (uint8_t) arg;

	Message_post(msg);
}

void TimeoutInit()
{
	Clock_Params clockParams;

	Clock_Params_init(&clockParams);
	clockParams.period = 0;       // one shot
	clockParams.startFlag = FALSE;

	_timer = Clock_create(0, 1000, &clockParams, 0);
}


/*
 * 设置某个类型的超时，如果超时，会发送该类型的Message
 * 由于目前只有一个计时器，调用TimeoutSet会将前一个超时停止
 */

void TimeoutSet(timeout_type_t msg)
{
	Clock_stop(_timer);
	Clock_setFunc(_timer, sendTimeMsg, (UArg) msg);
	Clock_setTimeout(_timer, m_timeout[(int) msg]);
	Clock_start(_timer);
}
