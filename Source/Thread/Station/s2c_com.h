/*
 * s2c_com.h
 *
 *  Created on: 2019-5-30
 *      Author: DELL
 */

#ifndef S2C_COM_H_
#define S2C_COM_H_

#define T1_PLATFORM_NUMS (3)
#define T2_PLATFORM_NUMS (5)

#define T1_PARKS    (1)
#define T2_PARKS    (2)

#define CAR_MODE_RUN (0)
#define CAR_MODE_PARK (1)
#define CAR_MODE_ERROR (2)

typedef struct
{
    uint8_t reserved;
    //干道编号
    uint8_t mainNo;
    //1级支道编号
    uint16_t firstNo;
    //2级支道编号
    uint8_t secondNo;
    //3级支道编号
    uint8_t thirdNo;
    //4级支道编号
    uint8_t fourthNo;
    //区域类型
    uint8_t areaType;
    //区域编号
    uint16_t areaNo;
    //子区域类型
    uint8_t subareaType;
    //道路特性
    uint8_t roadFeature;
    //距起始位置距离
    uint32_t distance;
}rfid_t;

typedef struct{
    rfid_t  rfid;
    uint16_t id;
    uint16_t rpm;
    uint8_t  mode;
    uint32_t timeStamp;
}carStatus_t;


typedef struct{
    uint8_t isUsed;
    uint16_t carId;
    uint32_t carPos;
    uint8_t carMode;
}stationTable_t;

typedef struct{
    uint32_t start;
    uint32_t end;
}parkArea_t;

typedef struct{
    uint8_t parkNums;
    uint8_t platNums;
    rfid_t parkRfid;
    uint32_t parkPos[5];
    uint32_t parkStart[5];
}cfgTable_t;


typedef struct{
    rfid_t reqRfid;
    uint32_t reqEndPointer;
    uint8_t reqRail;
    rfid_t cmpRfid;
    int32_t diffDist;
}seqCfgTable_t;

typedef struct{
    rfid_t rfidLeft;
    rfid_t rfidRight;
    uint32_t endPointer;
}adjCfgTable_t;


typedef struct{
    uint8_t isUsed;
    uint16_t carId;
    uint32_t dist;
}adjTable_t;

typedef struct{
    uint8_t isUsed;
    uint16_t carId;
    uint32_t dist;
}seqTable_t;

typedef struct{
    uint16_t carId;
    uint8_t type;
    uint8_t tId;
    /*
    uint8_t carMode;
    uint32_t carPos;
    */
}parkRequest_t;

typedef struct{
    uint16_t carId;
    rfid_t rfid;
}allotPacket_t;

typedef struct{
    uint16_t carId;
    rfid_t rfid;
    uint8_t rail;
}rid_t;

typedef struct{
    uint16_t carId;
    uint8_t type;
}doorCtrl_t;
#endif /* S2C_COM_H_ */


