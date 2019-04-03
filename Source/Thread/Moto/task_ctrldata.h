#ifndef __TASK_CTRLDATA_H
#define __TASK_CTRLDATA_H

#include <stdint.h>
#include "task_moto.h"


/*ͨѶ�����ź���ݽṹ*/
#pragma pack(1)
typedef struct
{
	uint8_t MotoSel;
	uint8_t ControlMode;
	uint8_t Gear;
	uint8_t Throttle;
	uint8_t Rail;
	uint8_t Brake;
    uint8_t RailState;
    uint16_t RPM;
    uint32_t KI;
    uint32_t KP;
    uint32_t KU;
    uint8_t AutoMode;    //0 手动 2自动
    uint8_t EnableChangeRail;
    uint8_t BrakeReady;
    uint8_t ChangeRailReady;
}ctrlData_t;



//void vCtrldataTask(void *param);
uint8_t CtrlGetBrake(void);
uint8_t CtrlGetRail(void);
uint8_t CtrlGetRailState(void);
uint8_t CtrlSetRailState(uint8_t sts);
extern ctrlData_t g_carCtrlData;
#endif
