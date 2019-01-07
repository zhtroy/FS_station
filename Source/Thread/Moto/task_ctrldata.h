#ifndef __TASK_CTRLDATA_H
#define __TASK_CTRLDATA_H

#include <stdint.h>

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
    uint8_t AutoMode;    //0 手动 2自动
}ctrlData;

//void vCtrldataTask(void *param);
uint8_t getBrake(void);
uint8_t getRail(void);
uint8_t getRailState(void);

#endif
