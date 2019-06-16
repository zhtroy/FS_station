/*
 * s2c_com.c
 *
 *  Created on: 2019-5-30
 *      Author: DELL
 */

#include "common.h"
#include "s2c_com.h"

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

#define S2C_ZCP_UART_DEV_NUM    (0)
#define S2C_ZCP_DEV_NUM         (0)

static const cfgTable_t constCfgTable[S2C_ROUTE_NUMS] = {
        {
                0x03,
                0x05,
                //站台区低速水平段：000101000000010B00053400
                0x00,0x01,0x01,0x00,0x00,0x00,0x01,0x0B,0x00,0x05,0x34,0x00,
                0x00000548,0x00000571,0x0000059A,0x00000000,0x00000000,
                0x00000534,0x0000055D,0x00000586,0x00000000,0x00000000
        },

        {
                0x01,
                0x05,
                //站台区低速水平段：000101010000010B00054600
                0x00,0x01,0x01,0x01,0x00,0x00,0x01,0x0B,0x00,0x05,0x46,0x00,
                0x0000055A,0x00000000,0x00000000,0x00000000,0x00000000,
                0x00000546,0x00000000,0x00000000,0x00000000,0x00000000
        },
};

static const adjCfgTable_t constAdjCfgTable[S2C_ADJ_NUMS] = {
        {
                //左调整区起点ID0：000100000000008100000000
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x81,0x00,0x00,0x00,0x00,
                //右调整区起点ID18：00010200000000C100000000
                0x00,0x01,0x02,0x00,0x00,0x00,0x00,0xC1,0x00,0x00,0x00,0x00,
                //调整区结束点距离
                0x00000140
        },
        {
                //左调整区起点ID56：00010000000000820007D000
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x82,0x00,0x07,0xD0,0x00,
                //右调整区起点ID55：00010100000080E20007D000
                0x00,0x01,0x01,0x00,0x00,0x00,0x80,0xE2,0x00,0x07,0xD0,0x00,
                //调整区结束点距离
                0x000008C0
        },
        {
                //左调整区起点ID52：00010100000000890005F000
                0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x89,0x00,0x05,0xF0,0x00,
                //右调整区起点ID51：00010101000000C90005F000
                0x00,0x01,0x01,0x01,0x00,0x00,0x00,0xC9,0x00,0x05,0xF0,0x00,
                //调整区结束点距离
                0x000006E0
        },
};

static const seqCfgTable_t constSeqCfgTable[S2C_SEQ_NUMS] = {
        {
                //分离区起点ID3:00010000000000420001E000
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x42,0x00,0x01,0xE0,0x00,
                //分离区终点
                0x000002DA,
                //轨道状态
                0x01,
                //分离区结束点ID41：00010000000000020002DA00
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0xDA,0x00,
                0x00000000
        },

        {
                //分离区起点ID3:00010000000000420001E000
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x42,0x00,0x01,0xE0,0x00,
                //分离区终点
                0x000002DA,
                //轨道状态
                0x02,
                //分离区结束点ID40：00010100000000020002DA00
                0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0xDA,0x00,
                0x00000000
        },
        {
                //分离区起点ID44:000101000000004200038400
                0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x42,0x00,0x03,0x84,0x00,
                //分离区终点
                0x00000424,
                //轨道状态
                0x01,
                //分离区结束点ID45：000101000000010B00042400
                0x00,0x01,0x01,0x00,0x00,0x00,0x01,0x0B,0x00,0x04,0x24,0x00,
                0x00000000
        },

        {
                //分离区起点ID44:000101000000004200038400
                0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x42,0x00,0x03,0x84,0x00,
                //分离区终点
                0x00000424,
                //轨道状态
                0x02,
                //分离区结束点ID46：000101010000010C00042400
                0x00,0x01,0x01,0x01,0x00,0x00,0x01,0x0C,0x00,0x04,0x24,0x00,
                0x00000000
        },
        {
                //分离区起点ID31:00010000000000410009D800
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x41,0x00,0x09,0xD8,0x00,
                //分离区终点
                0x00000AA0,
                //轨道状态
                0x01,
                //分离区结束点ID33：0001000000000001000AA000
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0A,0xA0,0x00,
                0x00000000
        },

        {
                //分离区起点ID31:00010000000000410009D800
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x41,0x00,0x09,0xD8,0x00,
                //分离区终点
                0x00000AA0,
                //轨道状态
                0x02,
                //分离区结束点ID32：0001010000000001000AA000
                0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x00,0x0A,0xA0,0x00,
                0x00000000
        },
};
static ZCPInstance_t s2cInst;
static Mailbox_Handle carStatusMbox;
static Mailbox_Handle parkMbox;
static Mailbox_Handle allotMbox;
static Semaphore_Handle allotSem;
static Mailbox_Handle doorMbox;
static Mailbox_Handle ridMbox;
static uint8_t stationStatus = STATION_NOT_READY;
static uint8_t stationCarNums = 0;
static stationTable_t **stationTablePtr;
static cfgTable_t **cfgTablePtr;
static adjCfgTable_t **adjCfgTablePtr;
static adjTable_t **adjTablePtr;
static adjTable_t **adjTableOldPtr;
static seqCfgTable_t **seqCfgTablePtr;
static seqTable_t **seqTablePtr;
static uint8_t routeNums = S2C_ROUTE_NUMS;
static uint8_t adjNums = S2C_ADJ_NUMS;
static uint8_t seqNums = S2C_SEQ_NUMS;

static void S2CGetInstance(ZCPInstance_t* pInst);
static uint32_t S2CGetDistance(rfid_t rfid);
static uint8_t S2CGetArea(rfid_t rfid);
static void S2CCarStatusProcTask(UArg arg0, UArg arg1);
static void S2CRequestIDTask(UArg arg0, UArg arg1);
static void S2CRequestParkTask(UArg arg0, UArg arg1);
static void S2CAllotParkTask(UArg arg0, UArg arg1);
static void S2CDoorCtrlTask(UArg arg0, UArg arg1);
static uint32_t S2CSubDist(uint32_t a,uint32_t b);
static S2CCfgTableInit(uint8_t routeNums,uint8_t adjNums,uint8_t seqNums);
static S2CStationTableInit(uint8_t routeNums,uint8_t adjNums,uint8_t seqNums);
static uint8_t S2CStationCheck(carStatus_t *carSts,
        stationTable_t *stationTable,
        cfgTable_t *cfgTablePtr,
        uint8_t route);
static void S2CMessageInit();
static uint8_t S2CGetStationStatus();
static uint8_t S2CGetCarNums();
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
            memcpy(&parkReq.routeId,recvPacket.data,sizeof(routeId_t));
            Mailbox_post(parkMbox,&parkReq,BIOS_NO_WAIT);
            break;

        case S2C_REQUEST_ID_CMD:
            rid.carId = recvPacket.addr;

            memcpy(&rid.rfid,recvPacket.data,sizeof(rfid_t)+1);
            Mailbox_post(ridMbox,&rid,BIOS_NO_WAIT);
            break;

        case S2C_LEAVE_STATION_CMD:
            parkReq.carId = recvPacket.addr;
            parkReq.type = S2C_LEAVE_STATION;
            memcpy(&parkReq.routeId,recvPacket.data,sizeof(routeId_t));
            Mailbox_post(parkMbox,&parkReq,BIOS_NO_WAIT);
            break;

        case S2C_DOOR_CONTROL_CMD:
            door.carId = recvPacket.addr;
            door.type = recvPacket.data[0];
            Mailbox_post(doorMbox,&door,BIOS_NO_WAIT);
            break;
        case S2C_CAR_STATUS_CMD:
            carSts.id = recvPacket.addr;
            memcpy(&carSts.rfid,recvPacket.data,sizeof(rfid_t)+3);
            Mailbox_post(carStatusMbox,&carSts,BIOS_NO_WAIT);
            break;
        case S2C_ALLOT_PARK_ACK:
            Semaphore_post(allotSem);
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
    Semaphore_Params semParams;

    carStatusMbox = Mailbox_create (sizeof (carStatus_t),S2C_MBOX_DEPTH, NULL, NULL);
    parkMbox = Mailbox_create (sizeof (parkRequest_t),S2C_MBOX_DEPTH, NULL, NULL);
    allotMbox = Mailbox_create (sizeof (allotPacket_t),S2C_MBOX_DEPTH, NULL, NULL);
    doorMbox = Mailbox_create (sizeof (doorCtrl_t),S2C_MBOX_DEPTH, NULL, NULL);
    ridMbox = Mailbox_create (sizeof (rid_t),S2C_MBOX_DEPTH, NULL, NULL);
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    allotSem = Semaphore_create(0, &semParams, NULL);
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

    routeNums = S2C_ROUTE_NUMS;
    adjNums = S2C_ADJ_NUMS;
    seqNums = S2C_SEQ_NUMS;

    S2CCfgTableInit(routeNums,adjNums,seqNums);
    S2CStationTableInit(routeNums,adjNums,seqNums);

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

    task = Task_create((Task_FuncPtr)S2CAllotParkTask, &taskParams, NULL);
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

}
/*****************************************************************************
 * 函数名称: void S2CGetInstance(ZCPInstance_t* pInst)
 * 函数说明: 获取S2C的ZCP句柄
 * 输入参数: 无
 * 输出参数:
 *      pInst：指针
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CGetInstance(ZCPInstance_t* pInst)
{
    pInst = &s2cInst;
}

uint32_t S2CGetDistance(rfid_t rfid)
{
    uint32_t dist;
    memcpy(&dist,&rfid.byte[8],3);
    return dist;
}

uint8_t S2CGetArea(rfid_t rfid)
{
    uint8_t Area;
    Area = rfid.byte[7] >> 6;

    return Area;
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
    uint8_t stationSts = 0;
    uint8_t carNums = 0;
    uint8_t carChecks = 0;
    uint8_t i;
    uint32_t dist;
    carStatus_t carSts;
    adjCfgTable_t *adjCfgData;
    adjTable_t *adjData;
    adjTable_t *adjDataOld;
    uint8_t adjOk = 0;
    seqCfgTable_t *seqCfgData;
    seqTable_t *seqData;
    uint8_t state;
    while(1)
    {
        stationSts = S2CGetStationStatus();

        if(STATION_NOT_READY == stationSts)
        {
            /*
             * 站台未被初始化设置，每2s提醒设置站台
             */
            LogMsg("Station is not Setting\r\n");
            carChecks = 0;
            Task_sleep(2000);
        }
        else if(STATION_IS_READY == stationSts)
        {
            /*
             * 站台已被初始化设置，等待初始化站台区域车辆
             */
            carNums = S2CGetCarNums();
            if(carNums > 0)
            {
                Mailbox_pend(carStatusMbox,&carSts,BIOS_WAIT_FOREVER);

                for(i=0;i<routeNums;i++)
                {

                    carChecks += S2CStationCheck(&carSts,
                            stationTablePtr[i],
                            cfgTablePtr[i],
                            i);
                }

                if(carChecks == carNums)
                {
                    /*
                     * 站台停靠区内检测到足够车辆，站台初始化结束
                     */
                    S2CSetStationStatus(STATION_INIT_DONE);
                }
            }
            else
            {
                /*
                 * 站台区域无车辆，无需初始化
                 */
                S2CSetStationStatus(STATION_INIT_DONE);
            }
        }
        else if(STATION_INIT_DONE == stationSts)
        {
            Mailbox_pend(carStatusMbox,&carSts,BIOS_WAIT_FOREVER);

            /*
             * 判断车辆是否在调整区内
             */
            adjOk = 0;
            for(i=0;i<adjNums;i++)
            {
                adjCfgData = adjCfgTablePtr[i];
                adjData = adjTablePtr[i];
                adjDataOld = adjTableOldPtr[i];
                /*
                 * 判断线路是否匹配
                 */
                state = memcmp(&adjCfgData->rfidLeft,&carSts.rfid,sizeof(rfid_t)-4);
                if(state != 0)
                {
                    state = memcmp(&adjCfgData->rfidRight,&carSts.rfid,sizeof(rfid_t)-4);
                }


                if(state == 0 && S2CGetDistance(carSts.rfid) < adjCfgData->endPointer)
                {
                    if(adjData->isUsed == 0)
                    {
                        /*
                         * 保存节点信息
                         * 当前节点尚未分配前车，更新节点信息
                         * 跳出循环
                         */
                        memcpy(&adjDataOld,&adjData,sizeof(adjTable_t));

                        adjData->isUsed = 1;
                        adjData->carId = carSts.id;
                        adjData->dist = S2CSubDist(S2CGetDistance(carSts.rfid),S2CGetDistance(adjCfgData->rfidLeft));
                        adjOk = 1;
                        break;
                    }
                    else
                    {
                        if(adjData->carId == carSts.id)
                        {
                            /*
                             * 同一辆车，更新和调节点的距离
                             * 跳出查询
                             */
                            adjData->dist = S2CSubDist(S2CGetDistance(carSts.rfid),S2CGetDistance(adjCfgData->rfidLeft));
                            adjOk = 1;
                            break;
                        }
                        else
                        {
                            /*
                             * 不同车辆，若离调节点更近，则更新节点信息
                             */
                            dist = S2CSubDist(S2CGetDistance(carSts.rfid),S2CGetDistance(adjCfgData->rfidLeft));

                            if(dist <= adjData->dist)
                            {
                                memcpy(&adjDataOld,&adjData,sizeof(adjTable_t));
                                adjData->carId = carSts.id;
                                adjData->dist = dist;
                                adjOk = 1;
                                break;
                            }
                        }
                    }
                }/*if(state == 0 && carSt*/
            }/*for(i=0;i<adjNums;i++)*/

            if(adjOk == 1)
                continue;


            for(i=0;i<seqNums;i++)
            {
                seqCfgData = seqCfgTablePtr[i];
                seqData = seqTablePtr[i];
                /*
                 * 判断线路是否匹配
                 */
                state = memcmp(&seqCfgData->cmpRfid,&carSts.rfid,sizeof(rfid_t)-4);

                if(state == 0)
                {
                    if(seqData->isUsed == 0)
                    {
                        /*
                         * 当前节点尚未分配前车，更新节点信息
                         * 跳出循环
                         */
                        seqData->isUsed = 1;
                        seqData->carId = carSts.id;
                        seqData->dist = S2CSubDist(S2CGetDistance(carSts.rfid),S2CGetDistance(seqCfgData->cmpRfid));
                        break;
                    }
                    else
                    {
                        if(seqData->carId == carSts.id)
                        {
                            /*
                             * 同一辆车，更新和分离节点的距离
                             * 跳出查询
                             */
                            seqData->dist = S2CSubDist(S2CGetDistance(carSts.rfid),S2CGetDistance(seqCfgData->cmpRfid));
                            break;
                        }
                        else
                        {
                            /*
                             * 不同车辆，若离调节点更近，则更新节点信息
                             * 跳出查询
                             */
                            dist = S2CSubDist(S2CGetDistance(carSts.rfid),S2CGetDistance(seqCfgData->cmpRfid));
                            if(dist <= seqData->dist)
                            {
                                seqData->carId = carSts.id;
                                seqData->dist = dist;
                                break;
                            }
                        }
                    }
                }/*if(state == 0 && carSt*/
            }/*for(i=0;i<seqNums;i++)*/

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
    uint32_t dist;
    uint8_t i;
    uint8_t state;
    adjCfgTable_t *adjCfgData;
    adjTable_t *adjData;
    adjTable_t *adjDataOld;
    seqCfgTable_t *seqCfgData;
    seqTable_t *seqData;
    ZCPUserPacket_t sendPacket;
    while(1)
    {
        Mailbox_pend(ridMbox,&rid,BIOS_WAIT_FOREVER);
        areaType = S2CGetArea(rid.rfid);
        dist = S2CGetDistance(rid.rfid);
        if(areaType == EREA_ADJUST_LEFT || areaType == EREA_ADJUST_RIGHT)
        {
            /*
             * 调整区
             */
            for(i=0;i<adjNums;i++)
            {
                adjCfgData = adjCfgTablePtr[i];
                adjData = adjTablePtr[i];
                adjDataOld = adjTableOldPtr[i];

                state = memcmp(&adjCfgData->rfidLeft,&rid.rfid,sizeof(rfid_t)-4);
                if(state != 0)
                {
                    state = memcmp(&adjCfgData->rfidRight,&rid.rfid,sizeof(rfid_t)-4);
                }

                if(state == 0 && dist < adjCfgData->endPointer)
                {
                    sendPacket.addr = rid.carId;
                    sendPacket.len = 3;
                    sendPacket.type = S2C_REQUEST_ID_ACK;
                    if(adjData->isUsed == 0)
                    {
                        /*
                         * 该调整点无前车
                         */
                        memset(&sendPacket.data[0],0,3);
                    }
                    else
                    {
                        /*
                         * 该调整点有前车
                         */
                        if(adjData->carId != rid.carId)
                        {
                            sendPacket.data[0] = 1;
                            memcpy(&sendPacket.data[1],&adjData->carId,2);
                        }
                        else if(adjDataOld->carId != rid.carId)
                        {
                            sendPacket.data[0] = 1;
                            memcpy(&sendPacket.data[1],&adjDataOld->carId,2);
                        }
                        {
                            /*
                             * 该调整点无前车
                             */
                            memset(&sendPacket.data[0],0,3);
                        }

                    }
                    ZCPSendPacket(&s2cInst,&sendPacket,NULL,BIOS_NO_WAIT);

                    break;
                }
            }
        }/*if(areaType == EREA_ADJUST)*/
        else if(areaType == EREA_SEPERATE)
        {
            /*
             * 分离区
             */
            for(i=0;i<seqNums;i++)
            {
                seqCfgData = seqCfgTablePtr[i];
                seqData = seqTablePtr[i];
                /*
                 * 判断线路(ID和轨道)是否匹配
                 */
                state = memcmp(&seqCfgData->reqRfid,&rid.rfid,sizeof(rfid_t)-4);

                if(state == 0 && rid.rail == seqCfgData->reqRail)
                {
                    sendPacket.addr = rid.carId;
                    sendPacket.len = 3;
                    if(seqData->isUsed == 0)
                    {
                        /*
                         * 分离点无前车
                         */
                        memset(&sendPacket.data[0],0,3);
                    }
                    else
                    {
                        if(seqData->carId != rid.carId)
                        {
                            sendPacket.data[0] = 1;
                            memcpy(&sendPacket.data[1],&seqData->carId,3);
                        }
                        {
                            /*
                             * 该调整点无前车
                             */
                            memset(&sendPacket.data[0],0,3);
                        }
                    }
                    ZCPSendPacket(&s2cInst,&sendPacket,NULL,BIOS_NO_WAIT);
                    break;
                }
            }
        }

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

    uint8_t i,j;
    uint8_t tid = 0;
    stationTable_t *stationTable;
    cfgTable_t *cfgData;
    parkRequest_t parkReq;
    uint8_t allotState;
    uint8_t allotStation = 0;
    allotPacket_t allotPacket;
    ZCPUserPacket_t sendPacket;
    while(1)
    {
        Mailbox_pend(parkMbox,&parkReq,BIOS_WAIT_FOREVER);

        /*
         * 确定所属站台路线
         */
        tid = 0xff;
        for(i=0;i<routeNums;i++)
        {
            cfgData = cfgTablePtr[i];
            if(0 == memcmp(&cfgData->parkRfid.byte[1],&parkReq.routeId,sizeof(routeId_t)))
            {
                tid = i;
                break;
            }
        }

        if(0xff == tid)
        {
            /*
             * 非站台路线申请
             */
            continue;
        }

        stationTable = stationTablePtr[tid];

        if(parkReq.type == S2C_LEAVE_STATION)
        {
            /*
             * 1.确认离站车辆停靠点
             * 2.分配停靠点给其它车辆
             * 3.发送离站响应
             */
            for(i=0;i<cfgData->parkNums;i++)
            {
                if(stationTable[i].carId == parkReq.carId)
                {
                    if(stationTable[cfgData->parkNums].isUsed == 1)
                    {
                        memcpy(&stationTable[i],&stationTable[cfgData->parkNums],sizeof(stationTable_t));
                        for(j=cfgData->parkNums+1;j<cfgData->platNums;j++)
                        {
                            memcpy(&stationTable[j-1],&stationTable[j],sizeof(stationTable_t));

                            if(stationTable[j].isUsed == 0)
                                break;

                            if(j == (cfgData->platNums-1))
                            {
                                /*
                                 * 队尾推入空数据
                                 */
                                memset(&stationTable[j],0,sizeof(stationTable_t));
                            }
                        }
                        /*
                         * 发送分配报文-独立任务
                         */
                        allotPacket.carId = parkReq.carId;
                        memcpy(&allotPacket.rfid,&cfgData->parkRfid,sizeof(rfid_t));
                        sendPacket.data[8] = (cfgData->parkStart[i] >> 16) & 0xff;
                        sendPacket.data[9] = (cfgData->parkStart[i] >> 8) & 0xff;
                        sendPacket.data[10] = (cfgData->parkStart[i]) & 0xff;
                        Mailbox_post(allotMbox,&allotPacket,BIOS_NO_WAIT);

                    }
                    else
                    {
                        /*
                         * 当前停靠点无车辆可分配
                         */
                        memset(&stationTable[i],0,sizeof(stationTable_t));
                    }

                    sendPacket.addr = parkReq.carId;
                    sendPacket.type = S2C_LEAVE_STATION_ACK;
                    sendPacket.len = 0;

                    if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
                    {
                        LogMsg("Send Failed!\r\n");
                    }

                    LogMsg("Car %4x leave Station %d!\r\n",parkReq.carId,i);

                    break;
                }/*if(stationTable[i]->carId*/
            }/*for(i=0...*/


        }/*if(parkReq->type == S2C_LEAVE_STATION)*/
        else if(parkReq.type == S2C_INTO_STATION)
        {
            /*
             * 1.确认是否有空余停靠点
             * 2.分配位置
             * 3.回复报文
             */
            allotState = ALLOT_NULL;
            for(i=0;i<cfgData->platNums;i++)
            {
                if(stationTable[i].isUsed == 0)
                {
                    /*
                     * 有空余站点
                     */
                    stationTable[i].isUsed = 1;
                    stationTable[i].carId = parkReq.carId;
                    stationTable[i].carMode = 0;
                    stationTable[i].carPos = 0;
                    if(i < cfgData->parkNums)
                    {
                        allotState = ALLOT_PARK;
                        allotStation = i;
                        LogMsg("Car-%x:T%d-P%d,A\r\n",parkReq.carId,tid,i);
                    }
                    else
                    {
                        allotState = ALLOT_PLAT;
                        LogMsg("Car-%x:T%d-P%d,B\r\n",parkReq.carId,tid,i);
                    }
                    break;
                }
                else
                {
                    /*
                     * 判断是否已申请过
                     */
                    if(parkReq.carId == stationTable[i].carId)
                    {
                        if(i < cfgData->parkNums)
                        {
                            allotState = 1;
                            allotStation = i;
                            LogMsg("Car-%x:T%d-P%d,C\r\n",parkReq.carId,tid,i);
                        }
                        else
                        {
                            allotState = 2;
                            LogMsg("Car-%x:T%d-P%d,D\r\n",parkReq.carId,tid,i);
                        }
                        break;
                    }
                }
            }

            if(allotState == 0)
            {
                /*
                 * 异常：队列已满，没有位置可以分配
                 */
            }
            else
            {
                if(allotState == 1)
                {
                    sendPacket.data[0] = 1;
                    memcpy(&sendPacket.data[1],&cfgData->parkRfid,sizeof(rfid_t));
                    sendPacket.data[9] = (cfgData->parkStart[allotStation] >> 16) & 0xff;
                    sendPacket.data[10] = (cfgData->parkStart[allotStation] >> 8) & 0xff;
                    sendPacket.data[11] = (cfgData->parkStart[allotStation]) & 0xff;
                }
                else
                {
                    sendPacket.data[0] = 0;
                    memset(&sendPacket.data[1],0,sizeof(rfid_t));
                }

                sendPacket.addr = parkReq.carId;
                sendPacket.type = S2C_INTO_STATION_ACK;
                sendPacket.len = 13;
                if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
                {
                    LogMsg("Send Failed!\r\n");
                }
            }


        }/*else if(parkReq->type == S2C_INTO_STATION)*/
        else
        {
            LogMsg("Error PARK Requeset Type!!\r\n");
        }

    }
}

void S2CAllotParkTask(UArg arg0, UArg arg1)
{
    allotPacket_t allotPacket;
    ZCPUserPacket_t sendPacket;
    uint8_t i;
    while(1)
    {
        Mailbox_pend(allotMbox,&allotPacket,BIOS_WAIT_FOREVER);

        for(i=0;i<3;i++)
        {
            /*
             * 重试3次，若3次都未收到响应，则认为失败
             */
            memcpy(&sendPacket.data[0],&allotPacket.rfid,sizeof(rfid_t));
            sendPacket.addr = allotPacket.carId;
            sendPacket.type = S2C_ALLOT_PARK_CMD;
            sendPacket.len = sizeof(rfid_t);
            if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
            {
                LogMsg("Send Failed!\r\n");
            }

            if(TRUE == Semaphore_pend(allotSem,200))
                break;
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
    uint8_t i,j;
    cfgTable_t *cfgData;
    stationTable_t *stationTable;
    ZCPUserPacket_t sendPacket;
    uint8_t carIsFound;
    uint8_t tId,pId;
    while(1)
    {
        Mailbox_pend(doorMbox,&door,BIOS_WAIT_FOREVER);
        /*
         * 找到车辆所属站台
         */
        carIsFound = 0;
        for(i=0;i<routeNums;i++)
        {
            cfgData = cfgTablePtr[i];
            stationTable = stationTablePtr[i];
            for(j=0;j<cfgData->parkNums;j++)
            {
                if(door.carId == stationTable[j].carId)
                {
                    tId = i;
                    pId = j;
                    carIsFound = 1;
                    break;
                }
            }

            if(carIsFound == 1)
                break;
        }

        if(carIsFound == 1)
        {
            if(door.type == S2C_OPEN_DOOR)
            {
                /*
                 *  1.开门
                 *  2.等待开门结束
                 *  3.发送开门状态
                 */
                sendPacket.data[1] = 1;
                LogMsg("Open door of station-T%d-P%d\r\n",tId,pId);
            }
            else if(door.type == S2C_CLOSE_DOOR)
            {
                /*
                 *  1.关门
                 *  2.等待开门结束
                 *  3.发送开门状态
                 */
                sendPacket.data[1] = 1;
                LogMsg("Open door of station-T%d-P%d\r\n",tId,pId);
            }
            sendPacket.addr = door.carId;
            sendPacket.type = S2C_DOOR_CONTROL_ACK;
            sendPacket.len = 2;
            sendPacket.data[0] = door.type;
            if(FALSE == ZCPSendPacket(&s2cInst, &sendPacket, NULL,BIOS_NO_WAIT))
            {
                LogMsg("Send Failed!\r\n");
            }
        }
        else
        {
            LogMsg("Error:Car is not in Station Park!!\r\n");
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

S2CCfgTableInit(uint8_t routeNums,uint8_t adjNums,uint8_t seqNums)
{

    uint8_t i;
    cfgTable_t *cfgTable;
    adjCfgTable_t *adjCfgTable;
    seqCfgTable_t *seqCfgTable;

    /*
     * 初始化站台配置指针
     */
    cfgTablePtr = malloc(routeNums*sizeof(void*));

    for(i=0;i<routeNums;i++)
    {
         cfgTable = malloc(sizeof(cfgTable_t));

         memcpy(cfgTable,&constCfgTable[i],sizeof(cfgTable_t));

         cfgTablePtr[i] = cfgTable;

    }

    /*
     * 初始化调整点指针
     */
    adjCfgTablePtr = malloc(adjNums*sizeof(void*));
    for(i=0;i<adjNums;i++)
    {
        adjCfgTable = malloc(sizeof(adjCfgTable_t));
        memcpy(adjCfgTable,&constAdjCfgTable[i],sizeof(adjCfgTable_t));

        adjCfgTablePtr[i] = adjCfgTable;

    }

    /*
     * 初始化分离点指针
     */
    seqCfgTablePtr = malloc(seqNums*sizeof(void*));
    for(i=0;i<seqNums;i++)
    {
        seqCfgTable = malloc(sizeof(seqCfgTable_t));
        memcpy(seqCfgTable,&constSeqCfgTable[i],sizeof(seqCfgTable_t));

        seqCfgTablePtr[i] = seqCfgTable;

    }
}

S2CStationTableInit(uint8_t routeNums,uint8_t adjNums,uint8_t seqNums)
{
    uint8_t i;
    uint8_t platNums;

    /*
     * 初始化站台停靠区指针
     */
    stationTablePtr = malloc(routeNums*sizeof(void*));

    for(i=0;i<routeNums;i++)
    {
        platNums = cfgTablePtr[i]->platNums;
        stationTablePtr[i] = malloc(platNums*sizeof(stationTable_t));
    }

    /*
     * 初始化调整区表指针
     */
    adjTablePtr = malloc(adjNums*sizeof(void*));
    adjTableOldPtr = malloc(adjNums*sizeof(void*));
    for(i=0;i<adjNums;i++)
    {
        adjTablePtr[i] = malloc(adjNums*sizeof(adjTable_t));
        adjTableOldPtr[i] = malloc(adjNums*sizeof(adjTable_t));
    }

    /*
     * 初始化分离区表指针
     */
    seqTablePtr = malloc(seqNums*sizeof(void*));
    for(i=0;i<seqNums;i++)
    {
        seqTablePtr[i] = malloc(seqNums*sizeof(seqTable_t));
    }
}


uint8_t S2CStationCheck(carStatus_t *carSts,
        stationTable_t *stationTable,
        cfgTable_t *cfgTablePtr,
        uint8_t route)
{
    uint8_t i,j;
    uint8_t result = 0;
    uint8_t platNums;
    uint8_t parkNums;
    uint32_t *parkPos;
    rfid_t *parkRfid;


    platNums = cfgTablePtr->platNums;
    parkNums = cfgTablePtr->parkNums;
    parkPos = cfgTablePtr->parkPos;
    parkRfid = &cfgTablePtr->parkRfid;

    if(memcmp(&carSts->rfid,parkRfid,sizeof(rfid_t)-4) == 0)
    {
        /*
         * 车辆处于停靠区
         */
        if(S2CGetDistance(carSts->rfid) > parkPos[0])
        {
            /*
             * 车辆超出第一个停靠点
             */
            LogMsg("T%d:Car %04x is out of parking area!!\r\n",route,carSts->id);
        }
        else if(S2CGetDistance(carSts->rfid) >= parkPos[parkNums-1])
        {
            /*
             * 车辆处于最后一个停靠点和第一个停靠点之间
             */
            for(i=0;i<parkNums;i++)
            {
                if(S2CGetDistance(carSts->rfid) == parkPos[i])
                {
                    /*
                     * 停靠点位置匹配
                     */
                    if(carSts->mode == CAR_MODE_PARK)
                    {
                        if(stationTable[i].isUsed == 0)
                        {
                            stationTable[i].carId = carSts->id;
                            stationTable[i].carPos = S2CGetDistance(carSts->rfid);
                            stationTable[i].carMode = CAR_MODE_PARK;
                            stationTable[i].isUsed = 1;
                            result = 1;
                            break;
                        }
                        else
                        {
                            /*
                             * 同一停靠点检测到多个车辆
                             * 不可能发生，除非误读RFID
                             */
                            if(carSts->id != stationTable[i].carId)
                            {
                                LogMsg("T%d:Park-%d Overlap!!\r\n",route,i);
                            }
                            break;
                        }
                    }
                    else
                    {
                        /*
                         * 车辆未处于停车状态
                         */
                        LogMsg("T%d:Car %04x is not parked!!\r\n",route,carSts->id);
                        break;
                    }
                }
                else
                {
                    /*
                     * 车辆未处于合适的停靠点
                     */
                    LogMsg("T%d:Car %04x is not in Park-%d!!\r\n",route,carSts->id, i);
                    break;
                }
            }/*for...*/
        }/*else if(carSts.rfid.distance >...*/
        else
        {
            /*
             * 车辆在停靠区，但是未在停靠点上
             * 1.查询是否已经存在队列中
             * 2.查询是否有剩余位置
             * 3.根据位置重新排序
             */
            for(i=parkNums;i<platNums;i++)
            {
                if(stationTable[i].isUsed == 1)
                {
                    if(stationTable[i].carId == carSts->id)
                    {
                        /*
                         * 同一车辆，则结束查询
                         */
                        break;
                    }
                    else
                    {
                        if(stationTable[i].carPos < S2CGetDistance(carSts->rfid))
                        {

                            if(stationTable[platNums-1].isUsed == 1)
                            {
                                /*
                                 * 当前站台没有多余的空位
                                 */
                                LogMsg("T%d:Station is Full!!!\r\n",route);
                                break;
                            }

                            /*
                             * 当前位置插入车辆
                             */
                            for(j=platNums-1;j>i;j--)
                            {
                                memcpy(&stationTable[j],&stationTable[j-1],sizeof(stationTable_t));
                            }

                            stationTable[i].carId = carSts->id;
                            stationTable[i].carPos = S2CGetDistance(carSts->rfid);
                            stationTable[i].carMode = CAR_MODE_PARK;
                            stationTable[i].isUsed = 1;
                            result = 1;
                            break;
                        }
                    }
                }
                else
                {
                    /*
                     * 停靠区未被分配
                     */
                    stationTable[i].carId = carSts->id;
                    stationTable[i].carPos = S2CGetDistance(carSts->rfid);
                    stationTable[i].carMode = CAR_MODE_PARK;
                    stationTable[i].isUsed = 1;
                    result = 1;
                    break;
                }
            }


        }
    }/*if(memcmp(carSts.rfid,T1_....*/

    return result;
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
