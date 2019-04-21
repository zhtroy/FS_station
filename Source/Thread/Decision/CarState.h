/*
 * CarState.h
 *
 *  Created on: 2019-4-17
 *      Author: zhtro
 */

#ifndef CARSTATE_H_
#define CARSTATE_H_

/*
 * 状态机状态
 */
typedef enum{
	cruising,
	straight,
	pre_curve,
	curving,
	uphill,
	pre_downhill,
	downhill,

	//并轨 合轨
	pre_seperate,
	seperate,
	enter_station,
	stop_station,
	leave_station,
	pre_merge,
	merge,

	idle,
	car_state_None
}car_state_t;


typedef enum
{
	Manual,
	Setting,
	Auto,
	ForceBrake
}car_mode_t;

#endif /* CARSTATE_H_ */
