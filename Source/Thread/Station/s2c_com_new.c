/*
 * s2c_com.c
 *
 *  Created on: 2019-5-30
 *      Author: DELL
 */

#include "common.h"
#include "Station\s2c_com_new.h"
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
                        0x00000572,
                        0x00000572,
                },
                {
                        0x0000054A,
                        0x0000054A,
                },
                {
                        0x00000522,
                        0x00000522,
                },
        },
        {
                {
                        0x00000532,
                        0x00000532,
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

static roadInformation_t *roadInfo;
static adjustZone_t *adjustZone;
static separateZone_t *separateZone;
static stationInformation_t *stationInfo;


static ZCPInstance_t s2cInst;
static Mailbox_Handle carStatusMbox;
static Mailbox_Handle parkMbox;
static Mailbox_Handle doorMbox;
static Mailbox_Handle ridMbox;

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
static void S2CShowStationLog();
static void S2CShowRoadLog();
static void S2CTimerTask(UArg arg0, UArg arg1);
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
            memcpy(&rid.rfid,recvPacket.data,sizeof(rfid_t)+5);

            /*请求ID先POST到状态处理任务，进行排队*/
            carSts.id = recvPacket.addr;
            carSts.rfid = rid.rfid;
            carSts.dist = rid.dist;
            carSts.rpm = 0;
            carSts.mode = CAR_MODE_RUN;
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
            memcpy(&carSts.rfid,recvPacket.data,sizeof(rfid_t)+7);
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

    ZCPInit(&s2cInst,S2C_ZCP_DEV_NUM,S2C_ZCP_UART_DEV_NUM);



    S2CStationDataInitial();

    S2CMessageInit();

    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;


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

void S2CDelStationPark(uint16_t carId)
{
    uint8_t i,j;
    for(i=0;i<termNums;i++)
    {
        for(j=0;j<stationInfo[i].parkNums;j++)
        {
            /*
             * 删除停靠点车辆
             */
            if(carId == stationInfo[i].carStation[j].id)
            {
                memset(&stationInfo[i].carStation[j],0,sizeof(carQueue_t));
            }
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
     * 离站条件
     * 1.车辆在站台队列中
     * 2.车辆道路信息不匹配，或车辆距离大于最后一个停靠点
     */
    for(i=0;i<termNums;i++)
    {
        index = S2CFindCarByID(carSts->id,stationInfo[i].carQueue);
        if(index >= 0)
        {
            index = S2CFindCarByID(carSts->id,stationInfo[i].carQueue);
            //if(0 == memcmp(&stationInfo[i].roadID,&carSts->rfid.byte[1],sizeof(roadID_t)+2))
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
                S2CDelStationPark(carSts->id);

                /*
                 * 删除分配队列车辆
                 */
                vector_erase(stationInfo[i].carQueue,index);

                LogMsg("Info:Car%x leave T%d-P%d\r\n",carSts->id,i,stationInfo[i].carQueue[index].pid);
            }
            break;
        }
    }

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
    uint8_t carIsSecB;
    int8_t index;
    uint8_t state;
    carStatus_t carSts;
    carQueue_t carQ;
    uint8_t isShowRoad = 0;
    roadInformation_t *roadFind;
    uint8_t areaType;
    while(1)
    {

        Mailbox_pend(carStatusMbox,&carSts,BIOS_WAIT_FOREVER);

        carQ.id = carSts.id;
        carQ.mode = carSts.mode;
        carQ.rpm = carSts.rpm;

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
                if(state == 0)
                {
                    /*
                     * 道路信息匹配，更新信息
                     */
                    if(carIsSecB)
                        carQ.pos = carSts.dist + roadInfo[i].sectionB;
                    else
                        carQ.pos = carSts.dist;

                    roadInfo[i].carQueue[index] = carQ;
                    carIsInserting = 0;
                }
                else
                {
                    /*
                     * 道路信息不匹配，从路线队列中移除车辆；
                     */
                    vector_erase(roadInfo[i].carQueue,index);
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

        areaType = S2CGetAreaType(carSts.rfid);
        if(areaType != EREA_ADJUST_RIGHT && areaType != EREA_ADJUST_LEFT)
        {
            /*
             * 车辆不处于调整区，遍历调整区队列，并删除该车辆
             */
            for(i=0;i<adjNums;i++)
            {
                index = S2CFindCarByID(carSts.id,adjustZone[i].carQueue);
                if(index >= 0)
                {
                    vector_erase(adjustZone[i].carQueue,index);
                    isShowRoad = 1;
                }
            }
        }

        if(isShowRoad == 1)
            S2CShowRoadLog();
    }
}

uint8_t S2CGetFrontCar(roadInformation_t *road,uint16_t carID,uint32_t dist,uint16_t *frontCar)
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
    uint8_t i;
    uint8_t carIsSecB;
    ZCPUserPacket_t sendPacket;
    roadInformation_t *roadFind;
    roadInformation_t *roadAdjust;
    roadID_t roadID;
    uint16_t frontCar;
    uint8_t state;
    uint8_t adjustZoneNums = 0;
    adjustZone_t *adjustZonePtr;
    carQueue_t carQ;
    uint8_t size;
    int8_t index;
    while(1)
    {
        Mailbox_pend(ridMbox,&rid,BIOS_WAIT_FOREVER);

        carQ.id = rid.carId;
        carQ.mode = 0;
        carQ.rpm = 0;

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
            continue;
        }

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
            adjustZonePtr = S2CGetAdjustZone(adjustZoneNums);

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
                    LogMsg("Adjust Queue Error!\r\n");
                }
                else
                {
                    if(index == 0)
                    {
                        state = 0;
                    }
                    else
                    {
                        state = 1;
                        frontCar = adjustZonePtr->carQueue[index-1].id;
                    }
                }

            }
            else
            {
                /*
                 * 调整区无前车
                 */
                for(i=0;i<roadNums;i++)
                {
                    if(0 == memcmp(&adjustZonePtr->leftRoadID,&roadInfo[i].roadID,sizeof(roadID_t)))
                    {
                        roadAdjust = &roadInfo[i];
                        break;
                    }
                }
                state = S2CGetFrontCar(roadAdjust,rid.carId,dist,&frontCar);
            }
        }
        else
        {
            state = S2CGetFrontCar(roadFind,rid.carId,dist,&frontCar);
        }

        if(state == 0)
        {
            /*
             * 无前车
             */
            memset(&sendPacket.data[0],0,3);
            LogMsg("ACK:car%x -> none\r\n",rid.carId);
        }
        else
        {


            sendPacket.data[0] = 1;
            memcpy(&sendPacket.data[1],&frontCar,2);
            LogMsg("ACK:car%x -> %x\r\n",rid.carId,frontCar);

        }
        S2CShowRoadLog();

        if(areaType == EREA_ADJUST_RIGHT)
        {
            /*
             * 右调整区返回对应左侧轨道(相邻轨道)
             */
            memcpy(&sendPacket.data[3],&roadAdjust->roadID,sizeof(roadID_t));
        }
        else
        {
            /*
             * 其它区域返回请求车辆轨道
             */
            memcpy(&sendPacket.data[3],&rid.rfid.byte[1],sizeof(roadID_t));
        }

        sendPacket.addr = rid.carId;
        //sendPacket.len = 3;
        sendPacket.len = 8;
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
                carQ.pid = stationFind->carQueue[size-1].pid + 1;


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


static void S2CShowStationLog()
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

static void S2CShowRoadLog()
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
