#include "task_ctrldata.h"
#include "task_moto.h"

ctrlData carCtrlData =
{
    FRONT_REAR,
    THROTTLE,
    DRIVE,
    0,
    0,
    0,
    0,     //railstate
    0,
    0,
    0,
    0
};
/*发送控制命令任务*/


uint8_t getBrake(void)
{
	return carCtrlData.Brake;
}

uint8_t getRail(void)
{
	return carCtrlData.Rail;
}

uint8_t getRailState(void)
{
	return carCtrlData.RailState;
}

uint8_t setRailState(uint8_t sts)
{
	carCtrlData.RailState = sts;
	return sts;
}


