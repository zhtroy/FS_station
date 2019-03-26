/*
 * test4GControl.h
 *
 *  Created on: 2019-1-10
 *      Author: zhtro
 */

#ifndef TEST4GCONTROL_H_
#define TEST4GCONTROL_H_

#include "stdint.h"

#define EPC_STRAIGHT   			(0x01)
#define EPC_PRE_CURVE  			(0x02)
#define EPC_CURVING    			(0x03)
#define EPC_UPHILL	   			(0x04)
#define EPC_PRE_DOWNHILL  		(0x05)
#define EPC_DOWNHILL  			(0x06)
#define EPC_PRE_SEPERATE  		(0x07)
#define EPC_SEPERATE  			(0x08)
#define EPC_ENTER_STATION  		(0x09)
#define EPC_STOP_STATION  		(0x0A)
#define EPC_LEAVE_STATION  		(0x0B)
#define EPC_PRE_MERGE 			(0x0C)
#define EPC_MERGE  				(0x0D)

#define EPC_AUXILIARY_TRACK_START (0x55)
#define EPC_AUXILIARY_TRACK_END   (0xAA)

#define FORCE_BRAKE				(150)

/*
 * 状态机
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

typedef enum{
	s_wait_photon,
	seperating,
	seperate_ok,
	seperate_cancel
}car_seperate_state_t;

typedef enum{
	m_wait_photon,
	merging,
	merge_ok
}car_merge_state_t;

typedef enum
{
	Manual,
	Setting,
	Auto,
	ForceBrake
}car_mode_t;

typedef enum
{
	seperate_wait_photon,
	seperate_wait_changerail,
	station_stop,
	merge_wait_photon,
	merge_wait_changerail,
	seperate_wait_enter_station,
	seperate_wait_stop_station,
	seperate_wait_leave_station,
	seperate_wait_pre_merge,
	seperate_wait_merge

}timeout_type_t;

/*
 * 延时
 */
uint32_t g_timeout[]=
{
		2000,
		3000,
		5000,
		2000,
		4000,
		20000,
		10000,
		10000,
		10000,
		10000
};
//#define TIMEOUT_seperate_wait_photon (1000)
//#define TIMEOUT_seperate_wait_changerail_complete (2000)

#endif /* TEST4GCONTROL_H_ */
