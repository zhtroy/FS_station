#ifndef __TASK_CAN_H
#define __TASK_CAN_H

#include <stdint.h>

//MOTO F CAN ID
#define MOTO_F_CANID1	0x10F8E3F3
#define MOTO_F_CANID2	0x10F8139A
#define MOTO_F_CANID3	0x10F8138D
#define MOTO_F_CANID4	0x10F8137B
//MOTO R CAN ID
#define MOTO_R_CANID1	0x10F8E4F3
#define MOTO_R_CANID2	0x10F8149A
#define MOTO_R_CANID3	0x10F8148D
#define MOTO_R_CANID4	0x10F8147B

#define CAN_DEV_0       (0)
#define MOTO_CAN_DEVNUM CAN_DEV_0

//Moto CAN控制数据结构
typedef struct
{
	uint8_t Gear;
	uint8_t ThrottleL;
	uint8_t ThrottleH;
	uint8_t Mode;
	uint8_t TorqueL;
	uint8_t TorqueH;
	uint8_t SpeedOrBreakL;
	uint8_t SpeedOrBreakH;
}canData;


enum motoSel
{
	FRONT_ONLY,
	REAR_ONLY,
	FRONT_REAR,
};
enum motoID
{
	MOTO_FRONT,
	MOTO_REAR,
};
enum motoMode
{
	THROTTLE,
	TORQUE,
	SPEED,
};
enum motoGear
{
	NONE,
	DRIVE,
	REVERSE,
	LOW,
};

/*通讯反馈数据结构*/
typedef struct
{
	uint8_t CarStatus;
	uint8_t MotoSel;
	uint8_t MotoId;
	uint8_t BreakMode;
	uint8_t Break;
	uint8_t Reserve1;
	uint8_t Reserve2;
	uint8_t Reserve3;
	//CAN ID=10F81X9A
	uint8_t Gear;
	uint8_t ThrottleL;
	uint8_t ThrottleH;
	uint8_t MotoMode;
	uint8_t RPML;
	uint8_t RPMH;
	uint8_t MotoTemp;
	uint8_t DriverTemp;
	//CAN ID=10F81X8D
	uint8_t VoltL;
	uint8_t VoltH;
	uint8_t CurrentL;
	uint8_t CurrentH;
	uint8_t DistanceL;
	uint8_t DistanceH;
	uint8_t ErrCodeL;
	uint8_t ErrCodeH;
	//CAN ID=10F81X7B
	uint8_t TorqueCtrlL;
	uint8_t TorqueCtrlH;
	uint8_t RPMCtrlL;
	uint8_t RPMCtrlH;
	uint8_t TorqueL;
	uint8_t TorqueH;
	uint8_t CanReserved1;
	uint8_t CanReserved2;
}motor_data_t;

#pragma pack(1)
typedef struct{
	motor_data_t motorDataF;
	motor_data_t motorDataR;
	uint8_t rfid;
	uint8_t mode;
	uint8_t brake;
	uint8_t railstate;
	uint8_t FSMstate;
	uint32_t rfidReadTime;

}fbdata_t;

#define DIFF_RPM_UPSCALE (4000)
#define DIFF_RPM_DWSCALE (-4000)

#define ADJ_THROTTLE_UPSCALE (10)
#define ADJ_THROTTLE_DWSCALE (-10)

#define MAX_THROTTLE_SIZE (55)
#define MIN_THROTTLE_SIZE (-355)

#define BREAK_THRESHOLD (-100)

#define MAX_BRAKE_SIZE (200)

#define BRAKE_THRO_RATIO (1)

uint16_t getRPM(void);


#endif
