#ifndef __TASK_CTRLDATA_H
#define __TASK_CTRLDATA_H

#include <stdint.h>

/*通讯控制信号数据结构*/
typedef struct
{
	uint8_t MotoSel;
	uint8_t ControlMode;
	uint8_t Gear;
	uint8_t Throttle;
	uint8_t Rail;
	uint8_t Brake;
    uint8_t RailState;
}ctrlData;

//void vCtrldataTask(void *param);
uint8_t getBrake(void);
uint8_t getRail(void);
uint8_t getRailState(void);

#endif
