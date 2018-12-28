#include "task_ctrldata.h"
#include "task_moto.h"

ctrlData carCtrlData =
{
    FRONT_REAR,
    THROTTLE,
    DRIVE,
    100,
    0,
    100
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
