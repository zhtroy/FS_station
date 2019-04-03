#include "task_ctrldata.h"


ctrlData_t g_carCtrlData =
{
	.MotoSel = FRONT_REAR,	//MotoSel
    THROTTLE,	//ControlMode
    NONE,		//Gear
    0,			//Throttle
    0,			//Rail
    0,			//Brake
    0,     		//RailState
    0,			//RPM
    0,			//KI
    0,			//KP
    0,			//KU
    0,			//AutoMode
    0,			//EnableChangeRail
    1,			//BrakeReady
    1			//ChangeRailReady
};
/*发送控制命令任务*/


uint8_t CtrlGetBrake(void)
{
	return g_carCtrlData.Brake;
}

uint8_t CtrlGetRail(void)
{
	return g_carCtrlData.Rail;
}

uint8_t CtrlGetRailState(void)
{
	return g_carCtrlData.RailState;
}

uint8_t CtrlSetRailState(uint8_t sts)
{
	g_carCtrlData.RailState = sts;
	return sts;
}



