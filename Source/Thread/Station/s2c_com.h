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
    uint8_t isPark;
    uint16_t carId;
    uint32_t carPos;
    uint8_t carMode;
}platformStatus_t;

typedef struct{
    uint32_t start;
    uint32_t end;
}parkArea_t;
#endif /* S2C_COM_H_ */
