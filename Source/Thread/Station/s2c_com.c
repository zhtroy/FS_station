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

#define S2C_ZCP_UART_DEV_NUM    (0)
#define S2C_ZCP_DEV_NUM (0)

static ZCPInstance_t s2cInst;
static Mailbox_Handle carStatusMbox;
static platformStatus_t t1StatusTable[T1_PLATFORM_NUMS];
static platformStatus_t t2StatusTable[T2_PLATFORM_NUMS];
static parkArea_t t1ParkArea[T1_PARKS];
static parkArea_t t2ParkArea[T1_PARKS];
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

    while(1)
    {
        ZCPRecvPacket(&s2cInst, &recvPacket, &timestamp, BIOS_WAIT_FOREVER);
        switch(recvPacket.type)
        {
        case 0x00:
            break;
        case 0x01:
            break;
        default:
            break;
        }

    }
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

    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;

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
    uint8_t i,j;
    carStatus_t carSts;
    platformStatus_t psTmp,psTmp1;
    while(1)
    {
        stationSts = S2CGetStationStatus();

        if(STATION_NOT_READY == stationSts)
        {
            /*
             * 站台未被初始化设置，每2s提醒设置站台
             */
            LogPuts("Station is not Setting\r\n");
            carChecks = 0;
            Task_sleep(2000);
        }
        else if(STATION_IS_READY == stationSts)
        {
            /*
             * 站台已被初始化设置，等待初始化站台区域车辆
             */
            carNums = S2CGetCarNums();
            if(carNums >= 0)
            {
                Mailbox_pend(carStatusMbox,&carSts,BIOS_WAIT_FOREVER);

                if(memcmp(carSts.rfid,T1_PARK_RFID,sizeof(rfid_t)-4) == 0)
                {
                    /*
                     * 车辆处于T1停靠区
                     */
                    if(carSts.rfid.distance > t1ParkArea[0].end)
                    {
                        /*
                         * 车辆超出第一个停靠点
                         */
                        LogPrintf("T1:Car %04x is out of parking area!!\r\n",carSts.id);
                    }
                    else if(carSts.rfid.distance > t1ParkArea[T1_PARKS-1].start)
                    {
                        /*
                         * 车辆处于最后一个停靠点和第一个停靠点之间
                         */
                        for(i=0;i<T1_PARKS;i++)
                        {
                            if(carSts.rfid.distance > t1ParkArea[i].start)
                            {
                                if(carSts.mode == CAR_MODE_PARK)
                                {
                                    if(t1StatusTable[i].isUsed == 0)
                                    {
                                        t1StatusTable[i].carId = carSts.id;
                                        t1StatusTable[i].carPos = carSts.rfid.distance;
                                        t1StatusTable[i].carMode = CAR_MODE_PARK;
                                        t1StatusTable[i].isUsed = 1;
                                        carChecks ++;
                                    }
                                    else
                                    {
                                        /*
                                         * 同一停靠点检测到多个车辆
                                         */
                                        if(carSts.id != t1StatusTable[i].carId)
                                        {
                                            LogPrintf("T1 Park-%d Overlap!!\r\n",i);
                                        }
                                    }
                                }
                                else
                                {
                                    /*
                                     * 车辆未处于停车状态
                                     */
                                    LogPrintf("T1:Car %04x is not parked!!\r\n",carSts.id);
                                }
                            }
                            else
                            {
                                /*
                                 * 车辆未处于合适的停靠点
                                 */
                                LogPrintf("T1:Car %04x is not in Park-%d!!\r\n",carSts.id, i);
                            }
                        }
                    }/*else if(carSts.rfid.distance >...*/
                    else
                    {
                        /*
                         * 车辆在停靠区，但是未在停靠点上
                         * 1.查询是否已经存在队列中
                         * 2.查询是否有剩余位置
                         * 3.根据位置重新排序
                         */
                        for(i=T1_PARKS;i<T1_PLATFORM_NUMS;i++)
                        {
                            if(t1StatusTable[i].isUsed == 1)
                            {
                                if(t1StatusTable[i].carId == carSts.id)
                                {
                                    /*
                                     * 同一车辆，则结束查询
                                     */
                                    break;
                                }
                                else
                                {
                                    if(t1StatusTable[i].carPos < carSts.rfid.distance)
                                    {

                                        if(t1StatusTable[T1_PLATFORM_NUMS-1].isUsed == 1)
                                        {
                                            /*
                                             * 当前站台没有多余的空位
                                             */
                                            LogPuts("T1 Station is Full!!!\r\n");
                                            break;
                                        }

                                        /*
                                         * 当前位置插入车辆
                                         */
                                        for(j=T1_PLATFORM_NUMS-1;j>i;j--)
                                        {
                                            memcpy(t1StatusTable[j],t1StatusTable[j-1],sizeof(platformStatus_t));
                                        }

                                        t1StatusTable[i].carId = carSts.id;
                                        t1StatusTable[i].carPos = carSts.rfid.distance;
                                        t1StatusTable[i].carMode = CAR_MODE_PARK;
                                        t1StatusTable[i].isUsed = 1;
                                        carChecks ++;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                /*
                                 * 停靠区未被分配
                                 */
                                t1StatusTable[i].carId = carSts.id;
                                t1StatusTable[i].carPos = carSts.rfid.distance;
                                t1StatusTable[i].carMode = CAR_MODE_PARK;
                                t1StatusTable[i].isUsed = 1;
                                carChecks ++;
                                break;
                            }
                        }


                    }
                }/*if(memcmp(carSts.rfid,T1_....*/
                else if(memcmp(carSts.rfid,T2_PARK_RFID,sizeof(rfid_t)-4) == 0)
                {
                    /*
                     * 车辆处于T2停靠区
                     */
                    if(carSts.rfid.distance > t2ParkArea[0].end)
                    {
                        /*
                         * 车辆超出第一个停靠点
                         */
                        LogPrintf("T2:Car %04x is out of parking area!!\r\n",carSts.id);
                    }
                    else if(carSts.rfid.distance > t2ParkArea[T2_PARKS-1].start)
                    {
                        /*
                         * 车辆处于最后一个停靠点和第一个停靠点之间
                         */
                        for(i=0;i<T2_PARKS;i++)
                        {
                            if(carSts.rfid.distance > t2ParkArea[i].start)
                            {
                                if(carSts.mode == CAR_MODE_PARK)
                                {
                                    if(t2StatusTable[i].isUsed == 0)
                                    {
                                        t2StatusTable[i].carId = carSts.id;
                                        t2StatusTable[i].carMode = CAR_MODE_PARK;
                                        t2StatusTable[i].isUsed = 1;
                                        carChecks ++;
                                    }
                                    else
                                    {
                                        /*
                                         * 同一停靠点检测到多个车辆
                                         */
                                        if(carSts.id != t2StatusTable[i].carId)
                                        {
                                            LogPrintf("T2 Park-%d Overlap!!\r\n",i);
                                        }
                                    }
                                }
                                else
                                {
                                    /*
                                     * 车辆未处于停车状态
                                     */
                                    LogPrintf("T2:Car %04x is not parked!!\r\n",carSts.id);
                                }
                            }
                            else
                            {
                                /*
                                 * 车辆未处于合适的停靠点
                                 */
                                LogPrintf("T2:Car %04x is not in Park-%d!!\r\n",carSts.id, i);
                            }
                        }
                    }
                    else
                    {
                        /*
                         * 车辆在停靠区，但是未在停靠点上
                         * 1.查询是否已经存在队列中
                         * 2.查询是否有剩余位置
                         * 3.根据位置重新排序
                         */
                        for(i=T2_PARKS;i<T2_PLATFORM_NUMS;i++)
                        {
                            if(t2StatusTable[i].isUsed == 1)
                            {
                                if(t2StatusTable[i].carId == carSts.id)
                                {
                                    /*
                                     * 同一车辆，则结束查询
                                     */
                                    break;
                                }
                                else
                                {
                                    if(t2StatusTable[i].carPos < carSts.rfid.distance)
                                    {

                                        if(t2StatusTable[T2_PLATFORM_NUMS-1].isUsed == 1)
                                        {
                                            /*
                                             * 当前站台没有多余的空位
                                             */
                                            LogPuts("T2 Station is Full!!!\r\n");
                                            break;
                                        }

                                        /*
                                         * 当前位置插入车辆
                                         */
                                        for(j=T2_PLATFORM_NUMS-1;j>i;j--)
                                        {
                                            memcpy(t2StatusTable[j],t2StatusTable[j-1],sizeof(platformStatus_t));
                                        }

                                        t2StatusTable[i].carId = carSts.id;
                                        t2StatusTable[i].carPos = carSts.rfid.distance;
                                        t2StatusTable[i].carMode = CAR_MODE_PARK;
                                        t2StatusTable[i].isUsed = 1;
                                        carChecks ++;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                /*
                                 * 停靠区未被分配
                                 */
                                t2StatusTable[i].carId = carSts.id;
                                t2StatusTable[i].carPos = carSts.rfid.distance;
                                t2StatusTable[i].carMode = CAR_MODE_PARK;
                                t2StatusTable[i].isUsed = 1;
                                carChecks ++;
                                break;
                            }
                        }
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
            Mailbox_pend(carStatusMbox,&carStatus,BIOS_WAIT_FOREVER);

        }

    }
}
