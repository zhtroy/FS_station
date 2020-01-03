/*
 * s2c_com.c
 *
 *  Created on: 2019-5-30
 *      Author: DELL
 */
#if ZIGBEE_WIFI==1
#include "common.h"
#include "Station\s2c_com_net.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/std.h>
#include "ZCP/zcp_driver.h"
#include "Message/Message.h"
#include <string.h>
#include <stdlib.h>
#include "vector.h"

#define S2C_ZCP_UART_DEV_NUM    (0)
#define S2C_ZCP_DEV_NUM         (0)

#define S2C_ROAD_NUMS (4)
#define S2C_SEP_NUMS  (3)
#define S2C_ADJ_NUMS  (3)
#define S2C_TERM_NUMS (2)
static const uint8_t roadNums = S2C_ROAD_NUMS;
static const uint8_t sepNums = S2C_SEP_NUMS;
static const uint8_t adjNums = S2C_ADJ_NUMS;
static const uint8_t termNums = S2C_TERM_NUMS;

static const roadInformation_t constRoadInfo[S2C_ROAD_NUMS] = {
        {
                //1:环形轨道,0:普通轨道
                0x01,
                //道路编号
                0x01,0x00,0x00,0x00,0x00,
                //B段偏移
                0,
                0x00000000,
        },
        {
                //1:环形轨道,0:普通轨道
                0x00,
                //道路编号
                0x01,0x01,0x00,0x00,0x00,
                //B段偏移
                0,
                0x00000000,
        },
        {
                //1:环形轨道,0:普通轨道
                0x01,
                //道路编号
                0x01,0x02,0x00,0x00,0x00,
                //B段偏移
                0,
                0x00000000,
        },
        {
                //1:环形轨道,0:普通轨道
                0x00,
                //道路编号
                0x01,0x01,0x01,0x00,0x00,
                //B段偏移
                0x00000000,
                0x00000000,
        }
};

static const adjustZone_t constAdjustZone[S2C_SEP_NUMS] = {
        {
                0,
                0x01,0x01,0x00,0x00,0x00,
                0x01,0x01,0x01,0x00,0x00,
                0x000005F0,
                0x000006E0,
                0,
        },
        {
                1,
                0x01,0x00,0x00,0x00,0x00,
                0x01,0x02,0x00,0x00,0x00,
                0x00000000,
                0x00000140,
                0,
        },
        {
                2,
                0x01,0x00,0x00,0x00,0x00,
                0x01,0x01,0x00,0x00,0x00,
                0x00000708,
                0x000008C0,
                0,
        },

};

static const separateZone_t constSeparateZone[S2C_ADJ_NUMS] = {
        {
                0x01,0x00,0x00,0x00,0x00,
                0x01,0x01,0x00,0x00,0x00,
                0x000001E0,
                0x000002DA,
        },
        {
                0x01,0x01,0x00,0x00,0x00,
                0x01,0x01,0x01,0x00,0x00,
                0x00000384,
                0x00000424,
        },
        {
                0x01,0x00,0x00,0x00,0x00,
                0x01,0x02,0x00,0x00,0x00,
                0x000009D8,
                0x00000AA0,
        },
};

static const stationInformation_t constStationInfo[S2C_TERM_NUMS] = {
        {
                0x01,0x01,0x00,0x00,0x00,
                0x01,
                0x0B,
                0x03,
                0x00000000,
                0x00000000,
        },
        {
                0x01,0x01,0x01,0x00,0x00,
                0x01,
                0x0B,
                0x01,
                0x00000000,
                0x00000000,
        },
};

static const park_t constParkInfo[S2C_TERM_NUMS][3] = {
        {
                {
                        0x00000564,
                        0x00000590,
                },
                {
                        0x0000053C,
                        0x00000564,
                },
                {
                        0x00000514,
                        0x0000053C,
                },
        },
        {
                {
                        0x0000051E,
                        0x00000546,
                },
                {
                        0x00000000,
                        0x00000000,
                },
                {
                        0x00000000,
                        0x00000000,
                },
        },
};

/*(220~227,163~170,20~27)*/
static criticalArea_t criticalArea[CRITICAL_AREA_NUMS] =
{
        /*起始位置,结束位置,车辆ID,车辆位置*/
        {
                220,227,0,0
        },
        {
                163,170,0,0
        },
        {
                20,27,0,0
        }
};

static collisionData_t collision_data;
static roadInformation_t *roadInfo;
static adjustZone_t *adjustZone;
static separateZone_t *separateZone;
static stationInformation_t *stationInfo;


static ZCPInstance_t s2cInst;
static Mailbox_Handle carStatusMbox;
static Mailbox_Handle parkMbox;
static Mailbox_Handle doorMbox;
static Mailbox_Handle ridMbox;
static Mailbox_Handle collisionMbox;

static uint8_t stationStatus = STATION_NOT_READY;
static uint8_t stationCarNums = 0;
static uint32_t timeStamp = 0;

static uint32_t S2CGetDistance(rfid_t rfid);
static uint8_t S2CGetAreaType(rfid_t rfid);
static void S2CCarStatusProcTask(UArg arg0, UArg arg1);
static void S2CRequestIDTask(UArg arg0, UArg arg1);
static void S2CRequestParkTask(UArg arg0, UArg arg1);
static void S2CDoorCtrlTask(UArg arg0, UArg arg1);
static void S2CMessageInit();
static uint8_t S2CGetAdjustZoneNums(rfid_t rfid);
static adjustZone_t* S2CGetAdjustZone(uint8_t nums);
static void S2CStationDataInitial();
static void S2CRemoveCarProcess(uint16_t carID);
static void S2CUpdateStation(carStatus_t *carSts);
static void S2CTimerTask(UArg arg0, UArg arg1);
static void S2CStationStopRequestTask(UArg arg0, UArg arg1);
static uint8_t S2CGetRoadSection(rfid_t rfid);
static uint8_t S2CGetFrontCar(roadInformation_t *road,uint16_t carID,uint32_t dist,uint16_t *frontCar);
/*****************************************************************************
 * 函数名称: void S2CRecvTask(UArg arg0, UArg arg1)
 * 函数说明: S2C接收任务
 * 输入参数:
 *      arg0~arg1:任务参数
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CRecvTask(UArg arg0, UArg arg1)
{
    ZCPUserPacket_t recvPacket;
    int32_t timestamp;
    parkRequest_t parkReq;
    doorCtrl_t door;
    rid_t rid;
    carStatus_t carSts;
    while(1)
    {
        ZCPRecvPacket(&s2cInst, &recvPacket, &timestamp, BIOS_WAIT_FOREVER);

        if((recvPacket.addr & 0x6000) != 0x6000)
            continue;

        switch(recvPacket.type)
        {
        case S2C_INTO_STATION_CMD:
            parkReq.carId = recvPacket.addr;
            parkReq.type = S2C_INTO_STATION;
            memcpy(&parkReq.roadID,recvPacket.data,sizeof(roadID_t));
            Mailbox_post(parkMbox,&parkReq,BIOS_NO_WAIT);
            break;

        case S2C_REQUEST_ID_CMD:


            rid.carId = recvPacket.addr;
            memcpy(&rid.rfid,recvPacket.data,sizeof(rfid_t)+7);

            /*请求ID先POST到状态处理任务，进行排队*/
            carSts.id = recvPacket.addr;
            carSts.rfid = rid.rfid;
            carSts.dist = rid.dist;
            carSts.rpm = 0;
            carSts.mode = CAR_MODE_RUN;
            carSts.rail = rid.rail;
            carSts.carMode = rid.carMode;

            Mailbox_post(carStatusMbox,&carSts,BIOS_NO_WAIT);
            Mailbox_post(ridMbox,&rid,BIOS_NO_WAIT);
            break;

        case S2C_LEAVE_STATION_CMD:
            parkReq.carId = recvPacket.addr;
            parkReq.type = S2C_LEAVE_STATION;
            memcpy(&parkReq.roadID,recvPacket.data,sizeof(roadID_t));
            Mailbox_post(parkMbox,&parkReq,BIOS_NO_WAIT);
            break;

        case S2C_DOOR_CONTROL_CMD:
            door.carId = recvPacket.addr;
            door.type = recvPacket.data[0];
            Mailbox_post(doorMbox,&door,BIOS_NO_WAIT);
            break;
        case S2C_CAR_STATUS_CMD:
            carSts.id = recvPacket.addr;
            memcpy(&carSts.rfid,recvPacket.data,sizeof(rfid_t)+9);
            Mailbox_post(carStatusMbox,&carSts,BIOS_NO_WAIT);
            break;
        default:
            break;
        }

    }
}

/*****************************************************************************
 * 函数名称: void S2CMessageInit()
 * 函数说明: S2C消息初始化
 *  1.初始化邮箱
 *  2.初始化信用量
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CMessageInit()
{

    carStatusMbox = Mailbox_create (sizeof (carStatus_t),S2C_MBOX_DEPTH, NULL, NULL);
    parkMbox = Mailbox_create (sizeof (parkRequest_t),S2C_MBOX_DEPTH, NULL, NULL);

    doorMbox = Mailbox_create (sizeof (doorCtrl_t),S2C_MBOX_DEPTH, NULL, NULL);
    ridMbox = Mailbox_create (sizeof (rid_t),S2C_MBOX_DEPTH, NULL, NULL);
    collisionMbox = Mailbox_create (sizeof (collisionData_t),S2C_MBOX_DEPTH, NULL, NULL);
}


/*****************************************************************************
 * 函数名称: void S2CTaskInit()
 * 函数说明: S2C任务初始化
 *  1.初始化ZCP；
 *  2.初始化S2C接收任务；
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CTaskInit()
{

    Task_Handle task;
    Task_Params taskParams;

    ZCPInit(&s2cInst,S2C_ZCP_DEV_NUM,S2C_ZCP_UART_DEV_NUM,STATION_ID);



    S2CStationDataInitial();

    S2CMessageInit();

    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 4096;


    task = Task_create((Task_FuncPtr)S2CCarStatusProcTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    task = Task_create((Task_FuncPtr)S2CRequestIDTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    task = Task_create((Task_FuncPtr)S2CRequestParkTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    task = Task_create((Task_FuncPtr)S2CDoorCtrlTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    task = Task_create((Task_FuncPtr)S2CRecvTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    task = Task_create((Task_FuncPtr)S2CStationStopRequestTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.priority = 4;
    task = Task_create((Task_FuncPtr)S2CTimerTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

#if 0
    taskParams.priority = 4;
    task = Task_create((Task_FuncPtr)S2CLogTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }
#endif
}

/*****************************************************************************
 * 函数名称: static void S2CStationDataInitial()
 * 函数说明: 站台数据初始化，初始化站台区域需要使用的各类数据结构
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
static void S2CStationDataInitial()
{
    uint8_t i;

    /*
     * 道路信息初始化
     */
    vector_grow(roadInfo,roadNums);
    memcpy(roadInfo,constRoadInfo,roadNums*sizeof(roadInformation_t));
    for(i=0;i<roadNums;i++)
    {
        vector_grow(roadInfo[i].carQueue,0);
    }

    /*
     * 调整区初始化
     */
    vector_grow(adjustZone,adjNums);
    memcpy(adjustZone,constAdjustZone,adjNums*sizeof(adjustZone_t));
    for(i=0;i<adjNums;i++)
    {
        vector_grow(adjustZone[i].carQueue,0);
    }

    /*
     * 分离区初始化
     */
    vector_grow(separateZone,sepNums);
    memcpy(separateZone,constSeparateZone,sepNums*sizeof(separateZone_t));

    /*
     * 站台区初始化
     */
    vector_grow(stationInfo,termNums);
    memcpy(stationInfo,constStationInfo,termNums*sizeof(stationInformation_t));
    for(i=0;i<termNums;i++)
    {
        vector_grow(stationInfo[i].park,stationInfo[i].parkNums);
        memcpy(stationInfo[i].park,&constParkInfo[i],stationInfo[i].parkNums*sizeof(park_t));
        vector_grow(stationInfo[i].carQueue,0);
        vector_grow(stationInfo[i].carStation,stationInfo[i].parkNums);
    }
}


int8_t S2CFindCarByID(uint16_t carID,carQueue_t *carQueue)
{
    int8_t index = -1;
    uint8_t i;
    uint8_t size = vector_size(carQueue);
    for(i=0;i<size;i++)
    {
        if(carID == carQueue[i].id)
        {
            index = i;
            break;
        }
    }
    return index;
}

#if 0
uint8_t S2CInsertCarByPosition(carQueue_t *carQueue,carQueue_t carQ)
{
    uint8_t i;
    if(vector_size(carQueue) == 0)
    {
        vector_push_back(carQueue,carQ);
        return 0;
    }

    for(i=0;i<vector_size(carQueue);i++)
    {
        if(carQ.pos > carQueue[i].pos)
        {
            vector_insert(carQueue,i,carQ);
            return i;
        }
    }

    vector_push_back(carQueue,carQ);
    return i;
}

uint8_t S2CRingInsertCarByPosition(carQueue_t *carQueue,carQueue_t carQ)
{
    uint8_t i,j;
    if(vector_size(carQueue) < 2)
    {
        vector_push_back(carQueue,carQ);
        return (vector_size(carQueue)-1);
    }

    for(i=0;i<vector_size(carQueue);i++)
    {
        if(i == 0)
            j = vector_size(carQueue)-1;
        else
            j = i-1;

        if(carQueue[j].pos > carQueue[i].pos)
        {
            if(carQ.pos > carQueue[i].pos && carQ.pos < carQueue[j].pos)
            {
                vector_insert(carQueue,i,carQ);
                return i;
            }
        }
        else
        {
            if(carQ.pos > carQueue[i].pos)
            {
                vector_insert(carQueue,i,carQ);
                return i;
            }
        }
    }
    return (0);
}
#endif



uint8_t S2CFindCarByPosition(carQueue_t *carQueue,uint32_t pos)
{
    uint8_t i;
    if(vector_size(carQueue) == 0)
    {
        return 0;
    }

    for(i=0;i<vector_size(carQueue);i++)
    {
        if(pos > carQueue[i].pos)
        {
            return i;
        }
    }
    return i;
}

uint8_t S2CRingFindCarByPosition(carQueue_t *carQueue,uint32_t pos)
{
    uint8_t i,j;
    uint8_t size;
    size = vector_size(carQueue);
    if(size == 0)
    {
        return (0);
    }

    if(size == 1)
    {
        if(carQueue[0].pos >= pos)
            return (1);
        else
            return (0);
    }

    for(i=0;i<size;i++)
    {
        if(i == 0)
            j = size-1;
        else
            j = i-1;

        /*
         * 环形排在前面的车，比后面的车位置靠后（翻转）
         */
        if(carQueue[j].pos < carQueue[i].pos)
        {
            if(pos > carQueue[i].pos || pos <= carQueue[j].pos)
            {
                return i;
            }
        }
        else
        {
            if(pos > carQueue[i].pos && pos <= carQueue[j].pos)
            {
                return i;
            }
        }
    }
    return (i);
}

uint8_t S2CRoadQueueInsertByPosition(roadInformation_t *road,carQueue_t carQ)
{
    uint8_t index;
    if(road->isRing == 0)
        index = S2CFindCarByPosition(road->carQueue,carQ.pos);
    else
        index = S2CRingFindCarByPosition(road->carQueue,carQ.pos);
    vector_insert(road->carQueue,index,carQ);
    return index;
}

uint8_t S2CStationQueueInsertByPosition(stationInformation_t *station,carQueue_t carQ)
{
    uint8_t index;
    index = S2CFindCarByPosition(station->carQueue,carQ.pos);
    vector_insert(station->carQueue,index,carQ);
    return index;
}

uint8_t S2CGetBSection(rfid_t rfid)
{
    if(rfid.byte[6] & 0x80)
        return 1;
    else
        return 0;
}


roadInformation_t *S2CFindAdjustRoad(uint8_t adjustArea,roadID_t *roadID,uint32_t pos)
{
    roadInformation_t * roadAdj=0;
    uint8_t i;
    roadID_t rid;
    for(i=0;i<adjNums;i++)
    {
        if(adjustArea == EREA_ADJUST_LEFT)
        {
            if((0 == memcmp(roadID,&adjustZone[i].leftRoadID,sizeof(roadID_t))) &&
                    (adjustZone[i].start <= pos) && (pos <= adjustZone[i].end))
            {
                rid = adjustZone[i].rightRoadID;
                break;
            }
        }
        else if(adjustArea == EREA_ADJUST_RIGHT)
        {
            if((0 == memcmp(roadID,&adjustZone[i].rightRoadID,sizeof(roadID_t))) &&
                    (adjustZone[i].start <= pos) && (pos <= adjustZone[i].end))
            {
                rid = adjustZone[i].leftRoadID;
                break;
            }
        }
    }

    for(i=0;i<roadNums;i++)
    {
        if(0 == memcmp(&rid,&roadInfo[i].roadID,sizeof(roadID_t)))
        {
            roadAdj = &roadInfo[i];
        }
    }

    return roadAdj;
}

roadID_t S2CFindSeparateRoad(roadID_t roadID,uint32_t pos)
{
    roadID_t rid;
    uint8_t i;
    for(i=0;i<sepNums;i++)
    {
        if(0 == memcmp(&roadID,&separateZone[i].leftRoadID,sizeof(roadID_t)) &&
                separateZone[i].start <= pos && pos <= separateZone[i].end)
        {
            rid = separateZone[i].rightRoadID;
            break;
        }
    }
    return rid;
}

void S2CDelStationPark(uint16_t carId,uint8_t stationId)
{
    uint8_t i;
    for(i=0;i<stationInfo[stationId].parkNums;i++)
    {
        /*
         * 删除停靠点车辆
         */
        if(carId == stationInfo[stationId].carStation[i].id)
        {
            memset(&stationInfo[stationId].carStation[i],0,sizeof(carQueue_t));
            LogMsg("Info:Clear Car%x from T%d\r\n",carId,i);
        }
    }
}

void S2CLevaveStationProcess(carStatus_t *carSts)
{
    uint8_t i;
    int8_t index;
    uint8_t carIsLeaving = 0;
    uint32_t dist;
    /*
     * 离站条件:
     * 车辆道路信息不匹配，或车辆距离大于最后一个停靠点
     */
    for(i=0;i<termNums;i++)
    {
        if(0 == memcmp(&stationInfo[i].roadID,&carSts->rfid.byte[1],sizeof(roadID_t)))
        {
            /*
             * 道路信息匹配
             */
            dist = S2CGetDistance(carSts->rfid);
            if(dist > stationInfo[i].park[0].point)
            {
                /*
                 * 距离超出最后一个停靠点
                 */
                carIsLeaving = 1;
            }
        }
        else
        {
            /*
             * 道路信息不匹配
             */
            carIsLeaving = 1;
        }



        if(carIsLeaving)
        {
            /*
             * 删除站点车辆
             */
            S2CDelStationPark(carSts->id,i);

            /*
             * 删除分配队列车辆
             */
            index = S2CFindCarByID(carSts->id,stationInfo[i].carQueue);
            if(index >= 0)
            {
                vector_erase(stationInfo[i].carQueue,index);
                LogMsg("Info:Car%x leave T%d-P%d\r\n",carSts->id,i,stationInfo[i].carQueue[index].pid);
            }

            carIsLeaving = 0;
        }
    }
}

uint8_t S2CCriticalAreaDetect(carStatus_t *carSts)
{
    uint8_t i;
    if(carSts->carMode != AUTO_MODE)
    {
        return 0;
    }
    /*关键区*/
    for(i=0;i<CRITICAL_AREA_NUMS;i++)
    {
        if(carSts->dist >= criticalArea[i].start && carSts->dist <= criticalArea[i].end)
        {
            /*关键区域存在一辆以上车辆*/
            if(criticalArea[i].carID != 0 && criticalArea[i].carID != carSts->id)
            {
                LogMsg("Critical Area collision detected:%x(%d),%x(%d)\r\n",
                        carSts->id,carSts->dist,
                        criticalArea[i].carID,criticalArea[i].carPos);
                return 1;
            }
            else
            {
                /*更新关键区域车辆信息*/
                criticalArea[i].carID = carSts->id;
                criticalArea[i].carPos = carSts->dist;
            }

        }
        else
        {
            /*车辆已经移出该区域，清除车辆*/
            if(criticalArea[i].carID == carSts->id)
            {
                criticalArea[i].carID = 0;
                criticalArea[i].carPos = 0;
            }
        }
    }
    return 0;
}

uint8_t S2CCarCollisionDetect(carStatus_t *carSts,roadInformation_t *roadFind)
{
    /*
     * --------- 碰撞风险 -------------------------------
     * 1) 站台区: 前车距离小于3.6米
     * 2) 调整区: 同一轨道前车距离小于6米
     * 3) 普通区: 同一轨道前车距离小于7米
     * 4) 关键区: 并轨区(220~227,163~170,20~27)内，超过1辆车
     */
    uint8_t roadSection;
    uint16_t frontCar;
    int8_t index_frontCar;
    carQueue_t frontCarInfo;
    int32_t dist_diff;
    uint8_t areaType;
    uint8_t isBSection;

    if(carSts->carMode != AUTO_MODE)
    {
        return 0;
    }

    /*道路内无前车*/
    if(0 == S2CGetFrontCar(roadFind,carSts->id,carSts->dist,&frontCar))
    {
        return 0;
    }

    index_frontCar = S2CFindCarByID(frontCar,roadFind->carQueue);
    frontCarInfo = roadFind->carQueue[index_frontCar];

    dist_diff = frontCarInfo.pos - carSts->dist;

    /*站台区*/
    roadSection = S2CGetRoadSection(carSts->rfid);
    if(frontCarInfo.roadSection == roadSection && SECTION_STATION == roadSection)
    {
        if(dist_diff >= 0 && dist_diff < MIN_DISTANCE_STATION)
        {
            LogMsg("Station Area collision detected:%x(%d),%x(%d)\r\n",
                    carSts->id,carSts->dist,
                    frontCarInfo.id,frontCarInfo.pos);

            return STATION_COLLISION_TYPE;
        }
        else
            return 0;
    }

    /*调整区*/
    areaType = S2CGetAreaType(carSts->rfid);
    //if(frontCarInfo.areaType == areaType &&
    //        frontCarInfo.adjustNums == adjustNums &&
    //        (EREA_ADJUST_LEFT == areaType || EREA_ADJUST_LEFT == areaType))
    if(EREA_ADJUST_LEFT == areaType || EREA_ADJUST_RIGHT == areaType)
    {
        if(dist_diff >= 0 && dist_diff < MIN_DISTANCE_ADJUST)
        {
            LogMsg("Adjust Area collision detected:%x(%d),%x(%d)\r\n",
                    carSts->id,carSts->dist,
                    frontCarInfo.id,frontCarInfo.pos);
            return ADJUST_COLLISION_TYPE;
        }
        else
            return 0;
    }

    /*普通区*/
    isBSection = S2CGetBSection(carSts->rfid);
    if(SECTION_NORMAL == roadSection)
    {
        /*环形轨道负值纠正*/
        if(roadFind->isRing && dist_diff < 0)
            dist_diff = dist_diff + S2C_RAIL_LENGTH;

        /*前车在B段，本车在A段，纠正距离差*/
        if(isBSection == 0 && frontCarInfo.isBSection == 1)
        {
            dist_diff = dist_diff - BSECTION_DIFF;
        }

        if(dist_diff >= 0 && dist_diff < MIN_DISTANCE_NORMAL)
        {
            LogMsg("Normal Area collision detected:%x(%d),%x(%d)\r\n",
                    carSts->id,carSts->dist,
                    frontCarInfo.id,frontCarInfo.pos);
            return NORMAL_COLLISION_TYPE;
        }

        else
            return 0;
    }

    return 0;
}

/*****************************************************************************
 * 函数名称: S2CCarStatusProcTask(UArg arg0, UArg arg1)
 * 函数说明: 车辆状态处理任务
 *      1.若站台未初始化完成，执行初始化动作，并初始化站台队列；
 *      2.初始化完成后，执行调整点和分离点的车辆管理。
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CCarStatusProcTask(UArg arg0, UArg arg1)
{
    uint8_t i;
    uint32_t distRfid;
    uint8_t carIsInserting;
    uint8_t carSepIsInserting;
    uint8_t carIsSecB;
    int8_t index;
    int8_t state;
    int8_t stateSep;
    carStatus_t carSts;
    carQueue_t carQ;
    uint8_t isShowRoad = 0;
    roadInformation_t *roadFind;
    roadInformation_t *roadSep;
    roadID_t roadSepID;
    uint8_t ajustNum;
    while(1)
    {

        Mailbox_pend(carStatusMbox,&carSts,BIOS_WAIT_FOREVER);


        isShowRoad = 0;

        /*
         * 车辆处于待删除模式
         */
        if(carSts.mode == CAR_MODE_REMOVE)
        {
            S2CRemoveCarProcess(carSts.id);
            S2CShowRoadLog();
            continue;
        }

        /*
         * 更新站点车辆
         */
        if(carSts.mode == CAR_MODE_PARK)
        {
            S2CUpdateStation(&carSts);
        }

        /*
         * 查找车辆路线信息
         */
        roadFind = 0;
        for(i=0;i<roadNums;i++)
        {
            state = memcmp(&carSts.rfid.byte[1],&roadInfo[i].roadID,sizeof(roadID_t));
            if(0 == state)
            {
                roadFind = &roadInfo[i];
                break;
            }
        }

        if(roadFind == 0)
        {
            LogMsg("Car %x:road ID is Wrong-%x%x%x!\r\n",carSts.id,
                    carSts.rfid.byte[0],
                    carSts.rfid.byte[1],
                    carSts.rfid.byte[2]);
            continue;
        }


        /*
         * 车辆离站处理
         */
        S2CLevaveStationProcess(&carSts);




        /*
         * 获取车辆距离信息
         */
        carIsSecB = S2CGetBSection(carSts.rfid);
        distRfid = S2CGetDistance(carSts.rfid);

        /*
         * 初始化队列信息
         */
        carQ.id = carSts.id;
        carQ.mode = carSts.mode;
        carQ.rpm = carSts.rpm;
        carQ.pos = 0;           /*默认值*/
        carQ.pid = 0;           /*默认值*/
        carQ.areaType = S2CGetAreaType(carSts.rfid);
        carQ.roadSection = S2CGetRoadSection(carSts.rfid);
        carQ.adjustNums = S2CGetAdjustZoneNums(carSts.rfid);
        carQ.isBSection = carIsSecB;
        carQ.carMode = carSts.carMode;
        carQ.rail = carSts.rail;
        memcpy(&carQ.roadID,&carSts.rfid.byte[1],sizeof(roadID_t));

        /*
         * 若车辆处于分离区的右侧轨道，获取分离区右轨信息
         */
        roadSep = 0;
        if(carQ.areaType == EREA_SEPERATE && carSts.rail == RIGHT_RAIL)
        {
            roadSepID = S2CFindSeparateRoad(roadFind->roadID,carSts.dist);
            for(i=0;i<roadNums;i++)
            {
                state = memcmp(&roadSepID,&roadInfo[i].roadID,sizeof(roadID_t));
                if(0 == state)
                {
                    roadSep = &roadInfo[i];
                    break;
                }
            }
            if(roadSep == 0)
            {
                LogMsg("Car %x is out of Seperate Zone\r\n");
                continue;
            }


            carSepIsInserting = 1;
        }

        /*
         * 碰撞风险检测
         */
        if(S2CCriticalAreaDetect(&carSts))
        {
            /*关键区域碰撞风险*/
            collision_data.carID = carSts.id;
            collision_data.type = 0;

            Mailbox_post(collisionMbox,&collision_data,BIOS_NO_WAIT);
        }
        else
        {
            collision_data.type = S2CCarCollisionDetect(&carSts,roadFind);

            if(collision_data.type == 0 && carQ.areaType == EREA_SEPERATE && carSts.rail == RIGHT_RAIL)
            {
                /*分离区车辆属于两条轨道，若主轨上无碰撞风险，则继续检测辅轨的碰撞风险*/
                collision_data.type =  S2CCarCollisionDetect(&carSts,roadSep);
            }

            if(collision_data.type > 0)
            {
                /*道路碰撞风险*/
                collision_data.carID = carSts.id;

                Mailbox_post(collisionMbox,&collision_data,BIOS_NO_WAIT);
            }
        }

        /*
         * --------------------- 路段队列处理   ------------------------------
         * 1.对应轨道已存在该车辆，则更新车辆信息；
         * 2.对应轨道不存在该车辆，则插入该车辆；
         * 3.其它轨道上存在该车辆，则从队列中删除该车辆；
         */
        carIsInserting = 1;

        for(i=0;i<roadNums;i++)
        {
            index = S2CFindCarByID(carSts.id,roadInfo[i].carQueue);
            if(index >= 0)
            {
                /*
                 * 找到车辆，判断道路信息是否匹配
                 */
                state = memcmp(&roadFind->roadID,&roadInfo[i].roadID,sizeof(roadID_t));

                stateSep = 1;
                if(state != 0 && carQ.areaType == EREA_SEPERATE && carSts.rail == RIGHT_RAIL)
                    stateSep = memcmp(&roadSep->roadID,&roadInfo[i].roadID,sizeof(roadID_t));

                if(state == 0 || stateSep == 0)
                {

                    if(carSts.carMode != AUTO_MODE)
                    {
                        /*
                         * 手动模式下，车辆位置关系重排
                         */
                        vector_erase(roadInfo[i].carQueue,index);
                    }
                    else
                    {
                        /*
                         * 非手动模式下，道路信息匹配，更新信息
                         */
                        if(carIsSecB)
                            carQ.pos = carSts.dist + roadInfo[i].sectionB;
                        else
                            carQ.pos = carSts.dist;

                        roadInfo[i].carQueue[index] = carQ;

                        if(state == 0)
                            carIsInserting = 0;
                        else
                            carSepIsInserting = 0;
                    }
                }
                else
                {
                    /*
                     * 道路信息不匹配，从路线队列中移除车辆；
                     */
                    vector_erase(roadInfo[i].carQueue,index);
                    if(carSts.carMode == AUTO_MODE)
                    {
                        LogMsg("\r\nStatus:Car%x(%d,%d) del %d from %x%x%x %04x\r\n",carSts.id,
                                        distRfid,
                                        carSts.dist,
                                        index,
                                        roadInfo[i].roadID.byte[0],
                                        roadInfo[i].roadID.byte[1],
                                        roadInfo[i].roadID.byte[2],
                                        distRfid);
                        isShowRoad = 1;
                    }
                }
            }
        }

        if(carIsInserting == 1)
        {
            /*
             * 车辆处于待插入状态，则在队列中插入车辆
             */
            if(carIsSecB)
                carQ.pos = carSts.dist + roadFind->sectionB;
            else
                carQ.pos = carSts.dist;

            index = S2CRoadQueueInsertByPosition(roadFind,carQ);
            if(carSts.carMode == AUTO_MODE)
            {
                LogMsg("\r\nStatus:Car%x(%d,%d) insert %d to %x%x%x %04x\r\n",carSts.id,
                                distRfid,
                                carSts.dist,
                                index,
                                roadFind->roadID.byte[0],
                                roadFind->roadID.byte[1],
                                roadFind->roadID.byte[2],
                                distRfid);
                isShowRoad = 1;
            }
        }

        if(carSepIsInserting == 1)
        {
            /*
             * 车辆处于待插入状态，则在队列中插入车辆
             */

            carQ.pos = carSts.dist;

            index = S2CRoadQueueInsertByPosition(roadSep,carQ);
            if(carSts.carMode == AUTO_MODE)
            {
                LogMsg("\r\nStatus:Car%x(%d,%d) insert %d to %x%x%x %04x\r\n",carSts.id,
                                distRfid,
                                carSts.dist,
                                index,
                                roadSep->roadID.byte[0],
                                roadSep->roadID.byte[1],
                                roadSep->roadID.byte[2],
                                distRfid);
                isShowRoad = 1;
            }
            carSepIsInserting = 0;
        }


        if(carQ.areaType != EREA_ADJUST_RIGHT && carQ.areaType != EREA_ADJUST_LEFT)
        {
            /*
             * 车辆不处于调整区，遍历调整区队列，并删除该车辆
             */
            for(i=0;i<adjNums;i++)
            {
                index = S2CFindCarByID(carSts.id,adjustZone[i].carQueue);
                if(index >= 0)
                {
                    if(carSts.carMode == AUTO_MODE)
                    {
                        if(carSts.dist >= adjustZone[i].end)
                        {
                            vector_erase(adjustZone[i].carQueue,index);
                            isShowRoad = 1;
                        }
                    }
                    else
                    {
                        vector_erase(adjustZone[i].carQueue,index);
                        isShowRoad = 1;
                    }

                }
            }
        }
        else if(carSts.carMode != AUTO_MODE)
        {
            /*
             * 车辆处于调整区，且车辆处于手动模式
             * 找到车辆所属调整区，并重新插入该车辆
             */
            ajustNum = S2CGetAdjustZoneNums(carSts.rfid);
            if(ajustNum < S2C_ADJ_NUMS)
            {
                if(adjustZone[ajustNum].start <= carSts.dist && carSts.dist <= adjustZone[ajustNum].end)
                {
                    index = S2CFindCarByID(carSts.id,adjustZone[ajustNum].carQueue);
                    if(index >= 0)
                    {
                        /*
                         * 若队列中存在该车辆，从队列中删除该车辆
                         */
                        vector_erase(adjustZone[ajustNum].carQueue,index);
                    }

                    /*
                     * 重新根据位置将车辆插入队列中
                     */
                    index = S2CFindCarByPosition(adjustZone[ajustNum].carQueue,carQ.pos);
                    vector_insert(adjustZone[ajustNum].carQueue,index,carQ);
                }
                else
                {
                    LogMsg("Manual Mode:%x Adjust scale error\r\n",carSts.id);
                }
            }
            else
            {
                LogMsg("Manual Mode:Adjust Number error\r\n",carSts.id);
            }
        }

        if(isShowRoad == 1)
            S2CShowRoadLog();
    }
}

static uint8_t S2CGetFrontCar(roadInformation_t *road,uint16_t carID,uint32_t dist,uint16_t *frontCar)
{
    volatile uint8_t size;
    int8_t index;
    uint8_t found;
    uint32_t frontCarDist;
    uint32_t distDiff;
    size = vector_size(road->carQueue);
    if(size == 0 || (size == 1 && carID == road->carQueue[0].id))
    {
        /*
         * 线路上无车辆
         */
        found = 0;
    }
    else
    {
        index = S2CFindCarByID(carID,road->carQueue);

        if(index < 0)
        {
            if(road->isRing == 0)
                index = S2CFindCarByPosition(road->carQueue,dist);
            else
                index = S2CRingFindCarByPosition(road->carQueue,dist);
        }

        if(index == 0)
        {
            /*
             * 车辆在最前方
             */
            if(road->isRing == 0)
            {
                /*
                 * 非环形轨道，前方无车
                 */
                found = 0;
            }
            else
            {
                /*
                 * 环形轨道，取最后一个车
                 */
                *frontCar = road->carQueue[size-1].id;
                frontCarDist = road->carQueue[size-1].pos;
                found = 1;

            }

        }
        else
        {
            *frontCar = road->carQueue[index-1].id;
            frontCarDist = road->carQueue[index-1].pos;
            found = 1;
        }
    }

    if(found == 0)
    {
        return 0;
    }
    else
    {
        if(dist > frontCarDist)
        {
            distDiff = S2C_RAIL_LENGTH + frontCarDist - dist;
        }
        else
        {
            distDiff = frontCarDist - dist;
        }

        if(distDiff > (S2C_MAX_FRONT_CAR_DISTANCE))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

/*****************************************************************************
 * 函数名称: S2CRequestIDTask(UArg arg0, UArg arg1)
 * 函数说明: 前车ID请求任务
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CRequestIDTask(UArg arg0, UArg arg1)
{
    rid_t rid;
    uint8_t areaType;
    uint32_t distRfid,dist;
    int8_t i;
    uint8_t carIsSecB;
    ZCPUserPacket_t sendPacket;
    roadInformation_t *roadFind;
    roadInformation_t *roadAdjust;
    roadID_t roadID;
    uint8_t adjustZoneNums = 0;
    adjustZone_t *adjustZonePtr;
    carQueue_t carQ;
    uint8_t size;
    int8_t index;
    frontCar_t frontCar;
    uint16_t tmp;
    while(1)
    {
        Mailbox_pend(ridMbox,&rid,BIOS_WAIT_FOREVER);

        carQ.id = rid.carId;
        carQ.mode = 0;
        carQ.rpm = 0;
        carQ.rail = rid.rail;
        memcpy(&carQ.roadID,&rid.rfid.byte[1],sizeof(roadID_t));
        /*
         * 确定车辆所属路线
         */
        areaType = S2CGetAreaType(rid.rfid);
        distRfid = S2CGetDistance(rid.rfid);
        carIsSecB = S2CGetBSection(rid.rfid);

        memcpy(&roadID,&rid.rfid.byte[1],sizeof(roadID_t));
        if(areaType == EREA_SEPERATE && rid.rail == RIGHT_RAIL)
        {
            roadID = S2CFindSeparateRoad(roadID,distRfid);
        }

        LogMsg("\r\n->%d",timeStamp);
        if(areaType == EREA_SEPERATE)
            LogMsg("\r\nSepr:");
        else if(areaType == EREA_ADJUST_RIGHT)
            LogMsg("\r\nRAdj:");
        else if(areaType == EREA_ADJUST_LEFT)
            LogMsg("\r\nLAdj:");
        else
            LogMsg("\r\nNorm:");

        LogMsg("Car%x(%d,%d,%d)Request ID.<%x%x%x %04x>\r\n",rid.carId,
                distRfid,
                rid.dist,
                rid.rail,
                rid.rfid.byte[1],
                rid.rfid.byte[2],
                rid.rfid.byte[3],
                rid.dist
                );

        roadFind = 0;
        for(i=0;i<roadNums;i++)
        {
            if(0 == memcmp(&roadID,&roadInfo[i].roadID,sizeof(roadID_t)))
            {
                roadFind = &roadInfo[i];
                break;
            }
        }
        if(roadFind == 0)
        {
            LogMsg("Warn:car%x EPC out of range\r\n",rid.carId);
            memset(&sendPacket.data[0],0,3);
            LogMsg("ACK:car%x -> none\r\n",rid.carId);
        }
        else
        {

            if(carIsSecB)
            {
                dist = rid.dist + roadFind->sectionB;
            }
            else
            {
                dist = rid.dist;
            }

            if(areaType == EREA_ADJUST_RIGHT || areaType == EREA_ADJUST_LEFT)
            {

                /*
                 * --------------------- 调整区队列处理   ------------------------------
                 * 1.车辆处于调整区，则将车辆放入对应队列，或者更新信息；
                 * 2.车辆处于非调整区，则将队列中的车辆删除；
                 */
                areaType = S2CGetAreaType(rid.rfid);


                adjustZoneNums = S2CGetAdjustZoneNums(rid.rfid);


                if(adjustZoneNums > S2C_ADJ_NUMS)
                {
                    LogMsg("Adjust Number Error!\r\n");
                    continue;
                }
                else
                {
                    adjustZonePtr = S2CGetAdjustZone(adjustZoneNums);
                    if(adjustZonePtr->start > rid.dist || adjustZonePtr->end < rid.dist)
                    {
                        LogMsg("Adjust Scale Error!\r\n");
                        continue;
                    }
                }

                index = S2CFindCarByID(rid.carId,adjustZonePtr->carQueue);
                carQ.pos = dist;
                if(index < 0)
                {
                    /*
                     * 车辆不存在，则将车辆推入队尾
                     */
                    vector_push_back(adjustZonePtr->carQueue,carQ);
                }
                else
                {
                    /*
                     * 车辆存在，则更新车辆信息
                     */
                    adjustZonePtr->carQueue[index] = carQ;
                }

                /*
                 * 车辆处于调整区
                 * 1.查询调整区队列是否有前车
                 * 2.若调整区内无前车，则查询对应左侧轨道(主轨)是否有前车
                 */
                //adjustZoneNums = S2CGetAdjustZoneNums(rid.rfid);
                //adjustZonePtr = S2CGetAdjustZone(adjustZoneNums);
                size = vector_size(adjustZonePtr->carQueue);
                if(size > 1)
                {
                    /*
                     * 调整区有前车
                     * ps:车辆在调整区申请时，已经处于调整区队列的队尾
                     */
                    index = S2CFindCarByID(rid.carId,adjustZonePtr->carQueue);
                    if(index < 0)
                    {
                        frontCar.state = 0;
                        LogMsg("Adjust Queue Error!\r\n");
                    }
                    else
                    {
                        if(index == 0)
                        {
                            frontCar.state = 0;
                        }
                        else
                        {
                            frontCar.state = 1;
                            frontCar.carID[0] = adjustZonePtr->carQueue[index-1].id;
                            frontCar.carID[1] = 0;

                            /*查找另外一条轨道上的前车*/
                            for(i=(index-2);i>=0;i--)
                            {
                                //if(adjustZonePtr->carQueue[index-1].rail != adjustZonePtr->carQueue[i].rail)
                                if(memcmp(&adjustZonePtr->carQueue[index-1].roadID,&adjustZonePtr->carQueue[i].roadID,sizeof(roadID_t)))
                                {
                                    frontCar.carID[1] = adjustZonePtr->carQueue[i].id;
                                    break;
                                }
                            }
                        }
                    }

                }
                else
                {
                    /*
                     * 调整区无前车，返回主道上的车辆
                     */
                    for(i=0;i<roadNums;i++)
                    {
                        if(0 == memcmp(&adjustZonePtr->leftRoadID,&roadInfo[i].roadID,sizeof(roadID_t)))
                        {
                            roadAdjust = &roadInfo[i];
                            break;
                        }
                    }
                    frontCar.state = S2CGetFrontCar(roadAdjust,rid.carId,dist,&tmp);
                    frontCar.carID[0] = tmp;
                    frontCar.carID[1] = 0;
                }
            }
            else
            {
                frontCar.state = S2CGetFrontCar(roadFind,rid.carId,dist,&tmp);
                frontCar.carID[0] = tmp;
                frontCar.carID[1] = 0;
            }

            if(frontCar.state == 0)
            {
                /*
                 * 无前车
                 */
                LogMsg("ACK:car%x -> none\r\n",rid.carId);
            }
            else
            {
                LogMsg("ACK:car%x -> %x,%x\r\n",rid.carId,frontCar.carID[0],frontCar.carID[1]);

            }
            S2CShowRoadLog();

            if(areaType == EREA_ADJUST_RIGHT)
            {
                /*
                 * 右调整区返回对应左侧轨道(相邻轨道)
                 */
                frontCar.roadID = roadAdjust->roadID;
            }
            else
            {
                /*
                 * 其它区域返回请求车辆轨道
                 */
                memcpy(&frontCar.roadID,&rid.rfid.byte[1],sizeof(roadID_t));
            }
        }

        sendPacket.addr = rid.carId;
        memcpy(sendPacket.data,&frontCar,sizeof(frontCar_t));
        sendPacket.len = sizeof(frontCar_t);
        sendPacket.type = S2C_REQUEST_ID_ACK;
        ZCPSendPacket(&s2cInst,&sendPacket,NULL,BIOS_NO_WAIT);
    }
}

/*****************************************************************************
 * 函数名称: S2CRequestParkTask(UArg arg0, UArg arg1)
 * 函数说明: 站台请求任务，实现站台的分配
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CRequestParkTask(UArg arg0, UArg arg1)
{

    uint8_t i;
    parkRequest_t parkReq;
    ZCPUserPacket_t sendPacket;
    stationInformation_t *stationFind;
    int8_t index;
    uint8_t size;
    carQueue_t carQ;
    uint8_t state;
    uint8_t tN;
    uint8_t stationFlag = 0;
    while(1)
    {
        Mailbox_pend(parkMbox,&parkReq,BIOS_WAIT_FOREVER);

        /*
         * 判断报文类型
         */
        if(parkReq.type != S2C_INTO_STATION)
            continue;

        LogMsg("Park:car%x request park\r\n",parkReq.carId);
        /*
         * 确定所属站台路线
         */
        stationFind = 0;
        for(i=0;i<termNums;i++)
        {
            state = memcmp(&parkReq.roadID,&stationInfo[i].roadID,sizeof(roadID_t));
            if(state == 0)
            {
                stationFind = &stationInfo[i];
                tN = i;
                break;
            }
        }

        if(stationFind == 0)
        {
            LogMsg("Warn:Car%x request park out of range:%x%x%x\r\n",parkReq.carId,
                    parkReq.roadID.byte[0],
                    parkReq.roadID.byte[1],
                    parkReq.roadID.byte[2]);
            continue;
        }

        carQ.id = parkReq.carId;
        carQ.mode = 0;
        carQ.rpm = 0;
        carQ.pos = 0;
        /*
         * 查找车辆
         */
        size = vector_size(stationFind->carQueue);
        if(size == 0)
        {
            for(i=stationFind->parkNums;i>0;i--)
            {

                if(stationFind->carStation[i-1].id != 0)
                     break;
            }
            carQ.pid = i;
            carQ.id = parkReq.carId;

            if(carQ.pid >= stationFind->parkNums)
            {
                carQ.pid = 0;
            }
            vector_push_back(stationFind->carQueue,carQ);
        }
        else
        {

            index = S2CFindCarByID(parkReq.carId,stationFind->carQueue);

            if(index < 0)
            {
                /*
                 * 未在站台队列中找到车辆，分配站台并推入队尾
                 */
                stationFlag = 0;

                for(i=0;i<stationFind->parkNums;i++)
                {
                    if(stationFind->carStation[i].id == stationFind->carQueue[size-1].id)
                    {
                        /*
                         * 站点队列中存在请求队列中的最后一个车辆，则按照站点队列的实际位置分配车辆
                         */
                        carQ.pid = stationFind->carStation[i].pid + 1;
                        stationFlag = 1;
                        break;
                    }
                }

                if(stationFlag == 0)
                {
                    carQ.pid = stationFind->carQueue[size-1].pid + 1;
                }



                if(carQ.pid >= stationFind->parkNums)
                {
                    carQ.pid = 0;
                }
                vector_push_back(stationFind->carQueue,carQ);
            }
            else
            {
                carQ.pid = stationFind->carQueue[index].pid;
            }
        }




        memset(sendPacket.data,0,13);
        sendPacket.data[0] = 1;
        memcpy(&sendPacket.data[2],&stationFind->roadID,sizeof(roadID_t)+2);
        sendPacket.data[9] = (stationFind->park[carQ.pid].trigger >> 16) & 0xff;
        sendPacket.data[10] = (stationFind->park[carQ.pid].trigger >> 8) & 0xff;
        sendPacket.data[11] = (stationFind->park[carQ.pid].trigger) & 0xff;
        sendPacket.addr = parkReq.carId;
        sendPacket.type = S2C_INTO_STATION_ACK;
        sendPacket.len = 13;
        LogMsg("Park:car%x -> T%d.P%d-%x\r\n",carQ.id,tN,carQ.pid,stationFind->park[carQ.pid].trigger);
        if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
        {
            LogMsg("Send Failed!\r\n");
        }
        S2CShowStationLog();
    }
}

/*****************************************************************************
 * 函数名称: S2CDoorCtrlTask(UArg arg0, UArg arg1)
 * 函数说明: 屏蔽门控制任务
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CDoorCtrlTask(UArg arg0, UArg arg1)
{
    doorCtrl_t door;
    uint8_t i;
    ZCPUserPacket_t sendPacket;

    doorStatus_t dSts;
    doorStatus_t * doorNode = 0;
    uint8_t state;
    int8_t index;
    stationInformation_t *stationFind;
    while(1)
    {
        state = Mailbox_pend(doorMbox,&door,100);

        if(TRUE == state)
        {
            /*
             * 确定所属站台路线
             */

            stationFind = 0;
            for(i=0;i<termNums;i++)
            {
                index = S2CFindCarByID(door.carId,stationInfo[i].carQueue);
                if(index >= 0)
                {
                    stationFind = &stationInfo[i];
                    dSts.tid = i;
                    break;
                }
            }

            if(stationFind == 0)
            {
                LogMsg("Car%x request door out of station\r\n",door.carId);
                continue;
            }



            dSts.pid = stationFind->carQueue[index].pid;
            dSts.carId = door.carId;
            dSts.type = door.type;
            vector_push_back(doorNode, dSts);
            if(door.type == S2C_OPEN_DOOR)
            {
                /*
                 *  1.开门
                 *  2.等待开门结束
                 *  3.发送开门状态
                 */

                LogMsg("Open door of station-T%d-P%d\r\n",dSts.tid,dSts.pid);
            }
            else if(door.type == S2C_CLOSE_DOOR)
            {
                /*
                 *  1.关门
                 *  2.等待关门结束
                 *  3.发送关门状态
                 */
                LogMsg("Close door of station-T%d-P%d\r\n",dSts.tid,dSts.pid);
            }

        }

        /*
         * 遍历doorNode,并根据门的状态发送响应报文
         * 周期：100ms
         */
#if 1
        if(0 == vector_empty(doorNode))
        {
            for(i=0;i<vector_size(doorNode);i++)
            {
                /*
                if(doorNode[i].type == S2C_OPEN_DOOR)
                {
                    sendPacket.data[1] = 1;
                }
                */
                sendPacket.data[1] = 1;
                sendPacket.addr = doorNode[i].carId;
                sendPacket.type = S2C_DOOR_CONTROL_ACK;
                sendPacket.len = 2;
                sendPacket.data[0] = doorNode[i].carId;
                vector_erase(doorNode,i);
                ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT);
            }
        }
#endif
    }
}

static void S2CTimerTask(UArg arg0, UArg arg1)
{
    while(1)
    {
        Task_sleep(100);
        timeStamp ++;
    }
}

static void S2CStationStopRequestTask(UArg arg0, UArg arg1)
{
    collisionData_t collisionInfo;
    uint8_t i,j;
    uint8_t size;
    int8_t index;
    ZCPUserPacket_t sendPacket;
    uint8_t retryNums;
    uint8_t isEnd;
    while(1)
    {
        Mailbox_pend(collisionMbox,&collisionInfo,BIOS_WAIT_FOREVER);
        retryNums = 0;
        do
        {
            isEnd = 1;
            if(collisionInfo.type == STATION_COLLISION_TYPE)
            {
                for(i=0;i<roadNums;i++)
                {
                    index = S2CFindCarByID(collisionInfo.carID,roadInfo[i].carQueue);
                    if(index > 0)
                    {
                        if(roadInfo[i].carQueue[index].carMode != STOP_MODE)
                        {
                            LogMsg("Collision Stop %x,type %d\r\n",collisionInfo.carID,collisionInfo.type);
                            sendPacket.addr = collisionInfo.carID;
                            sendPacket.len = 1;
                            sendPacket.type = S2C_REQUEST_STOP;
                            sendPacket.data[0] = collisionInfo.type;
                            ZCPSendPacket(&s2cInst,&sendPacket,NULL,BIOS_NO_WAIT);
                            Task_sleep(100);
                            isEnd = 0;
                            retryNums ++;
                        }
                        break;
                    }
                }

            }
            else
            {
                for(i=0;i<roadNums;i++)
                {
                    size = vector_size(roadInfo[i].carQueue);
                    for(j=0;j<size;j++)
                    {
                        if(roadInfo[i].carQueue[j].carMode != STOP_MODE)
                        {
                            LogMsg("Collision Stop %x,type %d\r\n",roadInfo[i].carQueue[j].id,collisionInfo.type);
                            sendPacket.addr = roadInfo[i].carQueue[j].id;
                            sendPacket.len = 1;
                            sendPacket.type = S2C_REQUEST_STOP;
                            sendPacket.data[0] = collisionInfo.type;
                            ZCPSendPacket(&s2cInst,&sendPacket,NULL,BIOS_NO_WAIT);
                            isEnd = 0;
                        }
                    }
                }
                retryNums ++;
            }
            Task_sleep(100);
        }while(retryNums < 5 && isEnd == 0);
    }
}


void S2CShowStationLog()
{
    uint8_t i,j;
    uint8_t size;
    /*
     * 显示站台队列
     */
    for(i=0;i<termNums;i++)
    {
        LogMsg("T%d:",i);
        for(j=0;j<stationInfo[i].parkNums;j++)
        {
            LogMsg(" %x",stationInfo[i].carStation[stationInfo[i].parkNums-1-j].id);
        }
        LogMsg("\r\n    ");
        size = vector_size(stationInfo[i].carQueue);
        for(j=0;j<size;j++)
            LogMsg("%x",stationInfo[i].carQueue[size-1-j].id);
        LogMsg("\r\n");
    }
}

void S2CShowRoadLog()
{
    uint8_t i,j;
    uint8_t size;
    /*
     * 显示道路队列
     */
    for(i=0;i<roadNums;i++)
    {
        LogMsg("R%x%x%x:",roadInfo[i].roadID.byte[0],
                roadInfo[i].roadID.byte[1],
                roadInfo[i].roadID.byte[2]);
        size = vector_size(roadInfo[i].carQueue);
        for(j=0;j<size;j++)
            LogMsg("->%x(%d)",roadInfo[i].carQueue[size-1-j].id,
                    roadInfo[i].carQueue[size-1-j].pos);
        LogMsg("\r\n");
    }

    /*
     * 显示调整区队列
     */
    for(i=0;i<adjNums;i++)
    {
        LogMsg("Adj%d:",adjustZone[i].adjustNums);
        size = vector_size(adjustZone[i].carQueue);
        for(j=0;j<size;j++)
            LogMsg("->%x(%d)",adjustZone[i].carQueue[size-1-j].id,
                    adjustZone[i].carQueue[size-1-j].pos);
        LogMsg("\r\n");
    }
}


void S2CSetStationStatus(uint8_t state)
{
    stationStatus = state;
}

void S2CSetCarNums(uint8_t nums)
{
    stationCarNums = nums;
}

static uint32_t S2CGetDistance(rfid_t rfid)
{
    uint32_t dist;
    dist = (rfid.byte[8]<<16) +
           (rfid.byte[9]<<8) +
            rfid.byte[10];
    return dist;
}

static uint8_t S2CGetAreaType(rfid_t rfid)
{
    uint8_t Area;
    Area = rfid.byte[7] >> 6;

    return Area;
}

static uint8_t S2CGetAdjustZoneNums(rfid_t rfid)
{
    uint8_t nums;
    nums = (rfid.byte[6] & 0x7f) >> 2;
    return nums;
}

static uint8_t S2CGetRoadSection(rfid_t rfid)
{
    uint8_t road_section;
    road_section = (rfid.byte[6] & 0x03);
    return road_section;
}

static adjustZone_t* S2CGetAdjustZone(uint8_t nums)
{
    return (&adjustZone[nums]);
}

void S2CRemoveCar(uint16_t carID)
{
    carStatus_t carSts;
    memset(&carSts,0,sizeof(carStatus_t));
    carSts.id = carID;
    carSts.mode = 0xff;
    Mailbox_post(carStatusMbox,&carSts,BIOS_NO_WAIT);
}

static void S2CRemoveCarProcess(uint16_t carID)
{
    uint8_t i;
    int8_t index;
    for(i=0;i<roadNums;i++)
    {
        index = S2CFindCarByID(carID,roadInfo[i].carQueue);
        if(index >= 0)
        {
            /*
             * 找到车辆
             */
            vector_erase(roadInfo[i].carQueue,index);
            LogMsg("Info:Car%x remove %d from road%x%x%x\r\n",carID,index,
                    roadInfo[i].roadID.byte[0],
                    roadInfo[i].roadID.byte[1],
                    roadInfo[i].roadID.byte[2]);

        }
    }

    for(i=0;i<adjNums;i++)
    {
        index = S2CFindCarByID(carID,adjustZone[i].carQueue);
        if(index >= 0)
        {
            /*
             * 找到车辆
             */
            vector_erase(adjustZone[i].carQueue,index);
        }
    }
}

static void S2CUpdateStation(carStatus_t *carSts)
{
    carQueue_t carQ;
    uint8_t i,j;
    uint32_t distRfid;
    uint8_t state;
    for(i=0;i<termNums;i++)
    {
        /*
         * 1.匹配路线
         * 2.匹配站点,更新车辆
         * 3.删除重复占用车辆
         */
        state = memcmp(&carSts->rfid.byte[1],&stationInfo[i].roadID,sizeof(roadID_t));

        if(state == 0)
        {
            for(j=0;j<stationInfo[i].parkNums;j++)
            {
                distRfid = S2CGetDistance(carSts->rfid);

                if(distRfid == stationInfo[i].park[j].point)
                {
                    carQ.id = carSts->id;
                    carQ.mode = carSts->mode;
                    carQ.pos = distRfid;
                    carQ.rpm = 0;
                    carQ.pid = j;

                    stationInfo[i].carStation[j] = carQ;
                }
                else
                {
                    /*
                     * 车辆ID匹配，停靠点位置不匹配
                     * 清除车辆
                     */
                    if(carSts->id == stationInfo[i].carStation[j].id)
                        memset(&stationInfo[i].carStation[j],0,sizeof(carQueue_t));
                }
            }
            break;
        }
    }
}

#endif
