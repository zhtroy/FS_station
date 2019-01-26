#include "task_ctrldata.h"


ctrlData g_carCtrlData =
{
    FRONT_REAR,	//MotoSel
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


uint8_t getBrake(void)
{
	return g_carCtrlData.Brake;
}

uint8_t getRail(void)
{
	return g_carCtrlData.Rail;
}

uint8_t getRailState(void)
{
	return g_carCtrlData.RailState;
}

uint8_t setRailState(uint8_t sts)
{
	g_carCtrlData.RailState = sts;
	return sts;
}



