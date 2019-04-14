/*
 * CarHsm.h
 *
 *  Created on: 2019-4-1
 *      Author: zhtro
 */

#ifndef CARHSM_H_
#define CARHSM_H_

#include "hsm.h"
#include "stdint.h"

typedef struct {
	Hsm super;
	State manual;
	State setting;
	State automode;
	State forcebrake;
}car_hsm_t;

enum CarEvents
{
	/*遥控事件 开始*/
	REMOTE_CHMODE_EVT,
	REMOTE_SET_MOTOR_EVT,
	REMOTE_SET_GEAR_EVT,
	REMOTE_SET_THROTTLE_EVT,
	REMOTE_CH_RAIL_EVT,
	REMOTE_SET_RAILSTATE_EVT,
	REMOTE_SET_BRAKE_EVT,

	/*RFID 开始*/
	RFID_EVT

};

/*
 * 以下是继承Msg的消息，加入需要的自定义数据，输入到HSM中
 *
 */

/*
 * 这个消息类型用于预先分配一块内存
 *
 * 其他的消息类型都不能大于这个类型
 */
typedef struct {
	Msg super;
	char mem[512];
}evt_placeholder_t;

/*
 *  遥控 (remote)
 *
 *  切换模式命令
 *
 */
typedef struct {
	Msg super;
	/*
	 * 模式跳转
	 * 0 手动模式
	 * 1 设置模式
	 * 2 自动模式
	 * 3 紧急制动模式
	 */
	uint8_t mode;
}evt_remote_chmode_t;

/*
 *  遥控 (remote)
 *
 *  电机选择
 *
 */
typedef struct {
	Msg super;
	/*
	 * 0 前轮
	 * 1 后轮
	 * 2 前后轮
	 */
	uint8_t mode;
}evt_remote_sel_motor_t;

/*
 * 遥控 (remote)
 *
 *	选择挡位
 */
typedef struct {
	Msg super;
	/*
	 * 0 空挡
	 * 1 前进
	 * 2 后退
	 * 3 慢速
	 */
	uint8_t gear;
}evt_remote_sel_gear_t;

/*
 * 遥控 (remote)
 *
 *	设置油门
 */
typedef struct {
	Msg super;
	uint8_t throttle;
}evt_remote_set_throttle_t;

/*
 * 遥控 (remote)
 *
 *	设置轨道状态
 */
typedef struct {
	Msg super;
	uint8_t state;
}evt_remote_set_railstate_t;

/*
 * 遥控 (remote)
 *
 *	设置刹车量
 */
typedef struct {
	Msg super;
	uint8_t brake;
}evt_remote_set_brake_t;

#endif /* CARHSM_H_ */
