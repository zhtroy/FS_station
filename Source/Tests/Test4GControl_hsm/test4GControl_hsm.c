/*
 * test4GControl_hsm.c
 *
 *	将test4GControl.c 中的switch case 状态机用hsm.h的方式实现
 *
 *
 *  Created on: 2019-3-27
 *      Author: zhtro
 */

#include "Message/Message.h"
#include "hsm.h"
#include "CarHsm.h"
#include "hsmUtil.h"
#include "stdlib.h"


static Void task4GControlMain_hsm(UArg a0, UArg a1)
{
	p_msg_t pMsg;
	evt_placeholder_t hsmEvt;

	while(1)
	{
		pMsg = Message_pend();

		/*
		 * TODO: 将输入数据记录在StatusData模块中
		 */

		/*
		 * 将pMsg翻译成HSM的事件 hsmEvt
		 */
		switch(pMsg->type)
		{
			case cell:
			{
				int data = atoi(&pMsg->data[1]);
				switch(pMsg->data[0])
				{
					case 'z': /*模式切换命令*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_CHMODE_EVT);
						EVT_CAST(&hsmEvt, evt_remote_chmode_t)->mode = pMsg->data[1] - '0';
						break;
					}

					case 'm': /*选择前驱，后驱，双驱*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_SET_MOTOR_EVT);
						EVT_CAST(&hsmEvt, evt_remote_sel_motor_t)->mode = data;
						break;
					}

					case 'g': /*选择挡位*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_SET_GEAR_EVT);
						EVT_CAST(&hsmEvt, evt_remote_sel_gear_t)->gear = data;
						break;
					}

					case 't': /*设置油门*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_SET_THROTTLE_EVT);
						EVT_CAST(&hsmEvt, evt_remote_set_throttle_t) ->throttle = data;
						break;
					}

					case 'r': /*开始变轨命令*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_CH_RAIL_EVT);
						break;
					}

					case 'R': /*设置当前轨道状态*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_SET_RAILSTATE_EVT);
						EVT_CAST(&hsmEvt, evt_remote_set_railstate_t)->state = data;
						break;
					}

					case 'b': /*设置刹车量*/
					{
						EVT_SETMSG(&hsmEvt, REMOTE_SET_BRAKE_EVT);
						EVT_CAST(&hsmEvt, evt_remote_set_brake_t)->brake = data;
						break;
					}

				}

			}
		}


		/*
		 * 输入event，驱动HSM运行
		 */

	}
}
