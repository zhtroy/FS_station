/*
 * CarHsm.c
 *
 * HSM定义，顶层状态
 *
 * 单个状态对应:
 * 		* car_hsm_t结构体中的的一个域
 * 		  命名方式为  父状态_状态 如automode_straight
 * 		* 一个状态处理函数
 * 		   状态处理函数命名方式为  AutomodeStraight
 *
 *  Created on: 2019-3-28
 *      Author: zhtro
 */

#include "CarHsm.h"
#include "hsmUtil.h"
#include "hsm.h"

Msg const *Top(car_hsm_t * me, Msg * msg)
{
	switch(msg->evt)
	{
		case START_EVT:
		{
			STATE_START(me, &me->manual);
			return 0;
		}
		case REMOTE_CHMODE_EVT:
		{
			switch(EVT_CAST(msg, evt_remote_chmode_t)->mode)
			{
				case 0:
					STATE_TRAN(me, &me->manual);
					break;
				case 1:
					STATE_TRAN(me, &me->setting);
					break;
				case 2:
					STATE_TRAN(me, &me->automode);
					break;
				case 3:
					STATE_TRAN(me, &me->forcebrake);
					break;
				default:
					break;
			}
			return 0;
		}
	}

	/*如果未处理该消息，返回msg*/
	return msg;
}

Msg const * TopManual(car_hsm_t * me, Msg* msg)
{
	switch(msg->evt)
	{
		case ENTRY_EVT:
		{
			//reset ctrl data
			return 0;
		}

		case REMOTE_CHMODE_EVT:
		{
			//EVT_CAST(msg, evt_remote_chmode_t)->mode;
			return 0;
		}
	}

	return msg;
}

void CarHsmCtor(car_hsm_t * me)
{
	HsmCtor((Hsm *)me, "Car", (EvtHndlr)Top);
	StateCtor(&me->manual, "manual", &((Hsm *)me)->top, (EvtHndlr)TopManual);
}

static car_hsm_t carHsm;

void CarHsmInit()
{
	CarHsmCtor(&carHsm);
	HsmOnStart((Hsm *) &carHsm);
}
