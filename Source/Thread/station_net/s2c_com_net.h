/*
 * s2c_com.h
 *
 *  Created on: 2019-5-30
 *      Author: DELL
 */

#ifndef S2C_COM_NET_H_
#define S2C_COM_NET_H_

#if ZIGBEE_WIFI==1
#include "common.h"

#define T1_PLATFORM_NUMS (3)
#define T2_PLATFORM_NUMS (5)

#define T1_PARKS    (1)
#define T2_PARKS    (2)

#define CAR_MODE_RUN (1)
#define CAR_MODE_PARK (0)
#define CAR_MODE_ERROR (2)

#define CAR_MODE_REMOVE (0xff)

#define STATION_ID (0x5001)
/*
 * 1.站台路线
 * 2.调整点数量
 * 3.分离点数量（1个分离区有两个分离点）
 */
#define S2C_ROUTE_NUMS  (2)
#define S2C_ADJ_NUMS    (3)
#define S2C_SEQ_NUMS    (6)

#define S2C_INTO_STATION_CMD    (0x01)
#define S2C_REQUEST_ID_CMD      (0x02)
#define S2C_DOOR_CONTROL_CMD    (0x03)
#define S2C_LEAVE_STATION_CMD   (0x04)
#define EVENT_V2C_PRE_ADJUST_REQUEST (0x05)
#define S2C_CAR_STATUS_CMD      (0x40)
#define S2C_ALLOT_PARK_ACK      (0X41)
#define S2C_REQUEST_STOP        (0x50)


#define S2C_ALLOT_PARK_CMD      (0x20)
#define S2C_INTO_STATION_ACK    (0x61)
#define S2C_REQUEST_ID_ACK      (0x62)
#define S2C_DOOR_CONTROL_ACK    (0x63)
#define S2C_LEAVE_STATION_ACK   (0x64)
#define EVENT_V2C_PRE_ADJUST_ACK            (0x65)      /*预调整区响应*/

#define PRE_ADJUST_DELAY_SEC (10)       /*预调整区延时间隔(s)*/


#define S2C_RAIL_LENGTH         (2810)
#define S2C_MAX_FRONT_CAR_DISTANCE (2000)

#define S2C_MBOX_DEPTH          (32)

#define EREA_ADJUST_LEFT             (0x02)
#define EREA_ADJUST_RIGHT            (0x03)
#define EREA_SEPERATE                (0x01)



#define S2C_CLOSE_DOOR          (0x01)
#define S2C_OPEN_DOOR           (0x00)

#define S2C_INTO_STATION        (0x01)
#define S2C_LEAVE_STATION       (0x02)

#define STATION_NOT_READY       (0x00)
#define STATION_IS_READY        (0x01)
#define STATION_INIT_DONE       (0x02)

#define ALLOT_NULL              (0x00)
#define ALLOT_PARK              (0x01)
#define ALLOT_PLAT              (0x02)

#define LEFT_RAIL               (0x02)
#define RIGHT_RAIL              (0x01)

#define MANUAL_MODE             (0x00)
#define SETTING_MODE            (0x01)
#define AUTO_MODE               (0x02)
#define STOP_MODE               (0x03)

#define SECTION_STATION         (0x01)
#define SECTION_NORMAL          (0x00)

#define MIN_DISTANCE_STATION    (36)
#define MIN_DISTANCE_ADJUST     (60)
#define MIN_DISTANCE_NORMAL     (70)

#define CRITICAL_AREA_NUMS         (3)

#define CRITICAL_AREA_TYPE      (0)
#define STATION_COLLISION_TYPE  (1)
#define ADJUST_COLLISION_TYPE   (2)
#define NORMAL_COLLISION_TYPE   (3)

#define BSECTION_DIFF           (160)
#pragma pack(1)
typedef struct
{
    uint8_t byte[12];
}rfid_t;
typedef struct{
    uint8_t byte[5];
}roadID_t;
typedef struct{
    uint16_t id;
    rfid_t  rfid;
    uint32_t dist;
    uint16_t rpm;
    uint8_t  mode;
    uint8_t  rail;
    uint8_t carMode;
    uint32_t timeStamp;
}carStatus_t;

typedef struct{
    uint16_t carId;
    uint8_t type;
    roadID_t roadID;
}parkRequest_t;

typedef struct{
    uint16_t carId;
    rfid_t rfid;
    uint32_t dist;
    uint8_t rail;
    uint8_t carMode;
    uint32_t timeStamp;
}rid_t;

typedef struct{
    uint16_t carId;
    uint8_t type;
}doorCtrl_t;

typedef struct{
    uint16_t carId;
    uint8_t type;
    uint8_t tid;
    uint8_t pid;
}doorStatus_t;



typedef struct{
    uint16_t id;
    uint16_t rpm;
    uint8_t mode;
    uint32_t pos;
    uint8_t pid;
    uint8_t roadSection;
    uint8_t areaType;
    uint8_t adjustNums;
    uint8_t isBSection;
    uint8_t carMode;
    uint8_t rail;
    roadID_t roadID;
}carQueue_t;

typedef struct{
    uint8_t isRing;
    roadID_t roadID;
    int32_t sectionB;
    carQueue_t *carQueue;
}roadInformation_t;

typedef struct{
    uint8_t adjustNums;
    roadID_t leftRoadID;
    roadID_t rightRoadID;
    uint32_t start;
    uint32_t end;
    carQueue_t *carQueue;
}adjustZone_t;

typedef struct{
    roadID_t leftRoadID;
    roadID_t rightRoadID;
    uint32_t start;
    uint32_t end;
}separateZone_t;

typedef struct{
    uint32_t trigger;
    uint32_t point;
}park_t;

typedef struct{
    roadID_t roadID;
    uint8_t roadType;
    uint8_t roadArea;
    uint8_t parkNums;
    park_t *park;
    carQueue_t * carQueue;
    carQueue_t * carStation;
}stationInformation_t;

typedef struct{
    uint32_t start;
    uint32_t end;
    uint16_t carID;
    uint32_t carPos;
}criticalArea_t;

typedef struct{
    uint8_t type;
    uint16_t carID;
}collisionData_t;

typedef struct{
    uint8_t state;
    uint16_t carID[2];
    roadID_t roadID;
}frontCar_t;

typedef struct{
    uint8_t status;
    uint8_t rfid[12];
}intoStationAck_t;

typedef struct{
    uint8_t operation;
    uint8_t status;
}doorAck_t;

typedef struct{
    uint8_t collision;
}stopRequest_t;

typedef struct{
    uint8_t goal_point[12];
    float run_time;
    uint16_t car_id;
    uint32_t receive_moment;
}preAdjustReq_t;

typedef struct{
    float except_time;
    uint32_t except_moment;
}preAdjustAck_t;

typedef struct{
    preAdjustReq_t preAdjust_req;
    preAdjustAck_t preAdjust_ack;
}preAdjustInfo_t;

#pragma pack()
void S2CTaskInit();


#endif

#endif /* S2C_COM_H_ */


