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
                -17,
                0x00000000,
        },
        {
                //1:环形轨道,0:普通轨道
                0x00,
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

static const ajustZone_t constAjustZone[S2C_SEP_NUMS] = {
        {
                0x01,0x00,0x00,0x00,0x00,
                0x01,0x02,0x00,0x00,0x00,
                0x00000000,
                0x00000140,
        },
        {
                0x01,0x00,0x00,0x00,0x00,
                0x01,0x01,0x00,0x00,0x00,
                0x000007D0,
                0x000008C0,
        },
        {
                0x01,0x01,0x00,0x00,0x00,
                0x01,0x01,0x01,0x00,0x00,
                0x000005F0,
                0x000006E0,
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
                        0x00000586,
                        0x00000586,
                },
                {
                        0x0000055D,
                        0x0000055D,
                },
                {
                        0x00000534,
                        0x00000534,
                },
        },
        {
                {
                        0x00000546,
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

static roadInformation_t *roadInfo;
static ajustZone_t *adjustZone;
static separateZone_t *separateZone;
static stationInformation_t *stationInfo;


static ZCPInstance_t s2cInst;
static Mailbox_Handle carStatusMbox;
static Mailbox_Handle parkMbox;
static Mailbox_Handle doorMbox;
static Mailbox_Handle ridMbox;

static uint8_t stationStatus = STATION_NOT_READY;
static uint8_t stationCarNums = 0;

static uint32_t S2CGetDistance(rfid_t rfid);
static uint8_t S2CGetArea(rfid_t rfid);
static void S2CCarStatusProcTask(UArg arg0, UArg arg1);
static void S2CRequestIDTask(UArg arg0, UArg arg1);
static void S2CRequestParkTask(UArg arg0, UArg arg1);
static void S2CDoorCtrlTask(UArg arg0, UArg arg1);
static uint32_t S2CSubDist(uint32_t a,uint32_t b);
static void S2CMessageInit();
static uint8_t S2CGetStationStatus();
static uint8_t S2CGetCarNums();
static void S2CStationDataInitial();
static void S2CLogTask(UArg arg0, UArg arg1);


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
    task = Task_create((Task_FuncPtr)S2CLogTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }
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
    memcpy(adjustZone,constAjustZone,adjNums*sizeof(ajustZone_t));

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
    }
}


int8_t S2CFindCarByID(uint16_t carID,carQueue_t *carQueue)
{
    int8_t index = -1;
    uint8_t i;
    for(i=0;i<vector_size(carQueue);i++)
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
    if(size < 2)
    {
        return (0);
    }

    for(i=0;i<size;i++)
    {
        if(i == 0)
            j = size-1;
        else
            j = i-1;

        if(carQueue[j].pos > carQueue[i].pos)
        {
            if(pos > carQueue[i].pos && pos < carQueue[j].pos)
            {
                return i;
            }
        }
        else
        {
            if(pos > carQueue[i].pos)
            {
                return i;
            }
        }
    }
    return (0);
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

uint8_t S2CGetAjustZone(rfid_t rfid)
{
    uint8_t state;
    state = (rfid.byte[7] >> 6);
    if(state == EREA_ADJUST_LEFT || state == EREA_ADJUST_RIGHT)
        return state;
    else
        return 0;
}

roadInformation_t *S2CFindAjustRoad(uint8_t isAjust,roadID_t *roadID,uint32_t pos)
{
    roadInformation_t * roadAdj=0;
    uint8_t i;
    roadID_t rid;
    for(i=0;i<adjNums;i++)
    {
        if(isAjust == EREA_ADJUST_LEFT)
        {
            if((0 == memcmp(roadID,&adjustZone[i].leftRoadID,sizeof(roadID_t))) &&
                    (adjustZone[i].start <= pos) && (pos <= adjustZone[i].end))
            {
                rid = adjustZone[i].rightRoadID;
                break;
            }
        }
        else if(isAjust == EREA_ADJUST_RIGHT)
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

void S2CLevaveStationProcess(carStatus_t *carSts)
{
    uint8_t i;
    int8_t index;
    uint8_t pid;
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
            if(0 == memcmp(&stationInfo[i].roadID,&carSts->rfid,sizeof(roadID_t)+2))
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
                 * 距离超出最后一个停靠点
                 */
                carIsLeaving = 1;
            }

            if(carIsLeaving)
            {
                pid = stationInfo[i].carQueue[index].pid;
                vector_erase(stationInfo[i].carQueue,index);
                LogMsg("Info:Car%x leave T%d-P%d\r\n",carSts->id,pid);
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
    uint8_t carChecks = 0;
    uint8_t i,j;
    uint32_t distRfid;
    uint8_t carIsFound;
    uint8_t carIsInserting;
    uint8_t carIsSecB;
    uint8_t carIsAjust;
    int8_t index,iAdj;
    uint8_t state;
    carStatus_t carSts;
    carQueue_t carQ;
    uint8_t isChecked;
    roadInformation_t *roadFind;
    roadInformation_t *roadAjust;
    while(1)
    {
        if(STATION_NOT_READY == stationStatus)
        {
            /*
             * 站台未被初始化设置，每2s提醒设置站台
             */
            LogMsg("Station is not Setting\r\n");
            carChecks = 0;
            Task_sleep(2000);
            continue;
        }

        if(stationCarNums == 0 || carChecks >= stationCarNums)
        {
            S2CSetStationStatus(STATION_INIT_DONE);
        }

        Mailbox_pend(carStatusMbox,&carSts,BIOS_WAIT_FOREVER);

        carQ.id = carSts.id;
        carQ.mode = carSts.mode;
        carQ.rpm = carSts.rpm;

        if(STATION_IS_READY == stationStatus)
        {
            /*
             * 初始化站台车辆
             */
            isChecked = 0;
            for(i=0;i<termNums;i++)
            {
                /*
                 * 1.匹配路线
                 * 2.匹配站点
                 * 3.判断是否有重复
                 */
                state = memcmp(&carSts.rfid.byte[1],&stationInfo[i].roadID,sizeof(roadID_t));

                if(state == 0)
                {
                    for(j=0;j<stationInfo[i].parkNums;j++)
                    {
                        distRfid = S2CGetDistance(carSts.rfid);
                        if(distRfid == stationInfo[i].park[j].point)
                        {
                            /*
                             * 查找是否有重复
                             */
                            index = S2CFindCarByID(carSts.id,stationInfo[i].carQueue);

                            if(index < 0)
                            {
                                /*
                                 * 未找到车辆，则插入
                                 */
                                carQ.pos = distRfid;
                                carQ.pid = j;
                                S2CStationQueueInsertByPosition(&stationInfo[i],carQ);
                            }

                            isChecked = 1;
                            break;
                        }
                    }
                }

                if(isChecked == 1)
                    break;
            }
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
         * 获取调整区对应路线信息
         */
        carIsAjust = S2CGetAjustZone(carSts.rfid);

        if(carIsAjust > 0)
        {
            roadAjust = S2CFindAjustRoad(carIsAjust,&roadFind->roadID,distRfid);

            if(roadAjust == 0)
            {
                LogMsg("Error:Ajust Zone cannot found\r\n");
                continue;
            }
        }
        else
        {
            roadAjust = 0;
        }

        /*
         * 遍历所有路线表，确认车辆是否已经存在
         */
        carIsFound = 0;
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


                    /*
                     * 判断调整区对应轨道是否存在该车辆
                     * 没有，则插入该车辆
                     */
                    if(carIsAjust > 0)
                    {
                        iAdj = S2CFindCarByID(carSts.id,roadAjust->carQueue);
                        if(iAdj < 0)
                        {
                            carQ.pos = carSts.dist + roadAjust->sectionB;
                            index = S2CRoadQueueInsertByPosition(roadAjust,carQ);
                            LogMsg("Info:Car%x insert %d of road%x%x%x\r\n",carSts.id,index,
                                    roadAjust->roadID.byte[0],
                                    roadAjust->roadID.byte[1],
                                    roadAjust->roadID.byte[2]);
                        }
                    }

                }
                else if(carIsAjust > 0 && 0 == memcmp(&roadAjust->roadID,&roadInfo[i].roadID,sizeof(roadID_t)))
                {
                    /*
                     * 道路信息匹配，为调整区另一侧车辆
                     * 更新信息
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
                    LogMsg("Info:Car%x del %d of road%x%x%x\r\n",carSts.id,index,
                            roadInfo[i].roadID.byte[0],
                            roadInfo[i].roadID.byte[1],
                            roadInfo[i].roadID.byte[2]);
                }

                carIsFound = 1;
            }
        }

        if(carIsInserting == 1)
        {
            /*
             * 车辆处于待插入状态，则在队列中插入车辆
             * 调整区车辆同时属于两条轨道
             */
            if(carIsSecB)
                carQ.pos = carSts.dist + roadFind->sectionB;
            else
                carQ.pos = carSts.dist;

            index = S2CRoadQueueInsertByPosition(roadFind,carQ);
            LogMsg("Info:Car%x insert %d of road%x%x%x\r\n",carSts.id,index,
                    roadFind->roadID.byte[0],
                    roadFind->roadID.byte[1],
                    roadFind->roadID.byte[2]);

            if(carIsAjust)
            {
                if(carIsAjust == EREA_ADJUST_LEFT)
                    carQ.pos = carSts.dist + roadAjust->sectionB;
                else
                    carQ.pos = carSts.dist;

                index = S2CRoadQueueInsertByPosition(roadAjust,carQ);
                LogMsg("Info:Car%x insert %d of road%x%x%x\r\n",carSts.id,index,
                        roadAjust->roadID.byte[0],
                        roadAjust->roadID.byte[1],
                        roadAjust->roadID.byte[2]);
            }
        }

        /*
         * 站台初始化阶段，统计站台区车辆数量
         */
        if(carIsFound == 0 && STATION_IS_READY == stationStatus)
        {
            carChecks++;
        }
    }
}

uint8_t S2CGetFrontCar(roadInformation_t *road,uint16_t carID,uint32_t dist,uint16_t *frontCar)
{
    uint8_t size;
    int8_t index;
    size = vector_size(road->carQueue);
    if(size == 0 || (size == 1 && carID == road->carQueue[0].id))
    {
        /*
         * 线路上无车辆
         */
        return 0;
        //memset(&sendPacket.data[0],0,3);
        //LogMsg("Info:car%x -> none\r\n",rid.carId);
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
                //memset(&sendPacket.data[0],0,3);
                //LogMsg("Info:car%x -> none\r\n",rid.carId);
                return 0;
            }
            else
            {
                /*
                 * 环形轨道，取最后一个车
                 */
                //sendPacket.data[0] = 1;
                //memcpy(&sendPacket.data[1],&road->carQueue[size-1].id,2);
                *frontCar = road->carQueue[size-1].id;
                return 1;
            }

        }
        else
        {
            //sendPacket.data[0] = 1;
            //memcpy(&sendPacket.data[1],&roadFind->carQueue[cindex-1].id,2);
            //LogMsg("Info:car%x -> %x\r\n",rid.carId,roadFind->carQueue[cindex-1].id);
            *frontCar = road->carQueue[index-1].id;
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
    while(1)
    {
        Mailbox_pend(ridMbox,&rid,BIOS_WAIT_FOREVER);

        if(stationStatus != STATION_INIT_DONE)
            continue;

        LogMsg("Info:Car%x Request ID\r\n",rid.carId);
        /*
         * 确定车辆所属路线
         */
        areaType = S2CGetArea(rid.rfid);
        distRfid = S2CGetDistance(rid.rfid);
        carIsSecB = S2CGetBSection(rid.rfid);

        memcpy(&roadID,&rid.rfid.byte[1],sizeof(roadID_t));
        if(areaType == EREA_SEPERATE && rid.rail == RIGHT_RAIL)
        {
            roadID = S2CFindSeparateRoad(roadID,distRfid);
        }

        roadFind = 0;
        for(i=0;i<roadNums;i++)
        {
            if(0 == memcmp(&roadID,&roadInfo[i].roadID,sizeof(roadID_t)))
            {
                roadFind = &roadInfo[i];
            }
        }
        if(roadFind == 0)
        {
            LogMsg("Warn:car%x EPC out of range\r\n",rid.carId);
            continue;
        }

        /*
         * 车辆处于右调整区，且当前轨道无前车，需要获取左侧轨道的前车
         */
        if(areaType == EREA_ADJUST_RIGHT)
        {
            roadAdjust = S2CFindAjustRoad(EREA_ADJUST_RIGHT,&roadFind->roadID,distRfid);
            if(roadAdjust == 0)
            {
                LogMsg("Warn:car%x out of Adjust zone\r\n",rid.carId);
                continue;
            }
        }


        if(carIsSecB)
        {
            dist = rid.dist + roadFind->sectionB;
        }
        else
        {
            dist = rid.dist;
        }

        state = S2CGetFrontCar(roadFind,rid.carId,dist,&frontCar);
        if(state == 0 && areaType == EREA_ADJUST_RIGHT)
        {
            /*
             * 车辆在右调整区，若调整区内无车，需申请主道车辆
             */
            state = S2CGetFrontCar(roadAdjust,rid.carId,rid.dist,&frontCar);
        }

        if(state == 0)
        {
            /*
             * 无前车
             */
            memset(&sendPacket.data[0],0,3);
            LogMsg("Info:car%x -> none\r\n",rid.carId);
        }
        else
        {
            sendPacket.data[0] = 1;
            memcpy(&sendPacket.data[1],&frontCar,2);
            LogMsg("Info:car%x -> %x\r\n",rid.carId,frontCar);
        }

        sendPacket.addr = rid.carId;
        sendPacket.len = 3;
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

    while(1)
    {
        Mailbox_pend(parkMbox,&parkReq,BIOS_WAIT_FOREVER);

        /*
         * 确定所属站台路线
         */
        if(stationStatus != STATION_INIT_DONE)
            continue;

        stationFind = 0;
        for(i=0;i<termNums;i++)
        {
            state = memcmp(&parkReq.roadID,&stationInfo[i].roadID,sizeof(roadID_t));
            if(state == 0)
            {
                stationFind = &stationInfo[i];
                break;
            }
        }

        if(stationFind == 0)
        {
            LogMsg("Car%x request park out of station\r\n",parkReq.carId);
            continue;
        }

        /*
         * 查找车辆
         */
        index = S2CFindCarByID(parkReq.carId,stationFind->carQueue);

        if(parkReq.type == S2C_LEAVE_STATION)
        {
            if(index < 0)
            {
                /*
                 * 未找到待离站车辆，认为车站已经离站
                 * 发送离站响应
                 */
            }
            else
            {
                /*
                 * 找到车辆，则从队列中删除该车辆
                 */
                vector_erase(stationFind->carQueue,index);
            }

            sendPacket.addr = parkReq.carId;
            sendPacket.type = S2C_LEAVE_STATION_ACK;
            sendPacket.len = 0;

            if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
            {
                LogMsg("Send Failed!\r\n");
            }
        }/*S2C_LEAVE_STATION*/
        else if(parkReq.type == S2C_INTO_STATION)
        {
            if(index < 0)
            {
                /*
                 * 未在站台队列中找到车辆，分配站台并推入队尾
                 */
                carQ.id = parkReq.carId;
                carQ.mode = 0;
                carQ.rpm = 0;
                carQ.pos = 0;
                size = vector_size(stationFind->carQueue);
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
            memcpy(&sendPacket.data[1],&stationFind->roadID,sizeof(roadID_t)+2);
            sendPacket.data[9] = (stationFind->park[carQ.pid].trigger >> 16) & 0xff;
            sendPacket.data[10] = (stationFind->park[carQ.pid].trigger >> 8) & 0xff;
            sendPacket.data[11] = (stationFind->park[carQ.pid].trigger) & 0xff;
            sendPacket.addr = parkReq.carId;
            sendPacket.type = S2C_INTO_STATION_ACK;
            sendPacket.len = 13;
            if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
            {
                LogMsg("Send Failed!\r\n");
            }
        }

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

        if(stationStatus != STATION_INIT_DONE)
            continue;

        if(TRUE == state)
        {
            /*
             * 确定所属站台路线
             */
            if(stationStatus != STATION_INIT_DONE)
                continue;

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

static void S2CLogTask(UArg arg0, UArg arg1)
{
    uint8_t i,j;
    uint8_t size;
    while(1)
    {
        Task_sleep(4000);

        if(stationStatus != STATION_INIT_DONE)
            continue;

        LogMsg("\r\n-----Station Queue Status------\r\n");
        /*
         * 显示站台队列
         */
        for(i=0;i<termNums;i++)
        {
            LogMsg("T%d:",i);
            size = vector_size(stationInfo[i].carQueue);
            for(j=0;j<size;j++)
                LogMsg(" %x",stationInfo[i].carQueue[j].id);
            LogMsg("\r\n");
        }

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
                LogMsg(" %x",roadInfo[i].carQueue[j].id);
            LogMsg("\r\n");
        }
    }
}

uint32_t S2CSubDist(uint32_t a,uint32_t b)
{
    uint32_t tmp;
    if(a >= b)
        tmp = a-b;
    else
        tmp = S2C_RAIL_LENGTH + a - b;
    return tmp;
}

uint8_t S2CGetStationStatus()
{
    return stationStatus;
}

void S2CSetStationStatus(uint8_t state)
{
    stationStatus = state;
}

uint8_t S2CGetCarNums()
{
    return stationCarNums;
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

static uint8_t S2CGetArea(rfid_t rfid)
{
    uint8_t Area;
    Area = rfid.byte[7] >> 6;

    return Area;
}

