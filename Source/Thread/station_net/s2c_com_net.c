/*
 * s2c_com.c
 *
 *  Created on: 2019-5-30
 *      Author: DELL
 */
#if ZIGBEE_WIFI==1
#define LOG_TAG "s2c"
#define LOG_LVL ELOG_LVL_DEBUG

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/std.h>
#include "station_net/s2c_com_net.h"
#include "Message/Message.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector.h"
#include "elog.h"
#include "hashtable.h"
#include "msg.h"
#include "uartStdio.h"
#include "easyflash.h"
#include "shell.h"
#include "common.h"
#include <time.h>

#define S2C_ZCP_UART_DEV_NUM    (0)
#define S2C_ZCP_DEV_NUM         (0)

#define S2C_ROAD_NUMS (4)
#define S2C_SEP_NUMS  (3)
#define S2C_ADJ_NUMS  (3)
#define S2C_TERM_NUMS (2)

static char log_buf[256];

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
roadInformation_t *roadInfo;
static adjustZone_t *adjustZone;
static separateZone_t *separateZone;
static stationInformation_t *stationInfo;


static Mailbox_Handle carStatusMbox;
static Mailbox_Handle parkMbox;
static Mailbox_Handle doorMbox;
static Mailbox_Handle ridMbox;
static Mailbox_Handle collisionMbox;
static Mailbox_Handle preAdjustMbox;
static Clock_Handle clockConnectHeart;

static uint8_t stationStatus = STATION_NOT_READY;
static uint8_t stationCarNums = 0;
static uint32_t timeStamp = 0;

static uint32_t getDistance(rfid_t rfid);
static uint8_t getAreaType(rfid_t rfid);
static void S2CCarStatusProcTask(UArg arg0, UArg arg1);
static void S2CRequestIDTask(UArg arg0, UArg arg1);
static void S2CRequestParkTask(UArg arg0, UArg arg1);
static void S2CDoorCtrlTask(UArg arg0, UArg arg1);
static void messageInit();
static uint8_t getAdjustZoneNums(rfid_t rfid);
static adjustZone_t* getAdjustZone(uint8_t nums);
static void stationDataInit();
static void removeCarProcess(uint16_t carID);
static void updateStation(carStatus_t *carSts);
static void S2CStationStopRequestTask(UArg arg0, UArg arg1);
static uint8_t getRoadSection(rfid_t rfid);
static uint8_t getFrontCar(roadInformation_t *road,uint16_t carID,uint32_t dist,carQueue_t *frontCar);
static void showRoadLog();
static void showStationLog();
static int on_serverconnect(SOCKET s, uint16_t id);
static int on_serverdisconnect(SOCKET s, uint16_t id);
static void S2CpreAdjustTask(UArg arg0, UArg arg1);
extern uint32_t gettime(void);

#define SERVER_MAX_LISTHEN_NUMS (10)

static HashTable *_socket_id_table;
static HashTable *_socket_id_stats;

/*compare pointer address*/
static int hashintkeycmp(const void* a ,const void * b)
{
	/*if(*((uint16_t*)a) == *((uint16_t*)b))*/
	/*{*/
		/*return 0;*/
	/*}*/
    if(a == b)
    {
        return 0;
    }
	else
	{
		return -1;
	}
}

static void msgServerInit(uint16_t myid, uint16_t port)
{
	msg_init(myid);

	//_sockettable
	HashTableConf htc;

	hashtable_conf_init(&htc);

	htc.hash        = GENERAL_HASH;
	htc.key_length  = sizeof(uint16_t);   //evt is int
	htc.key_compare = hashintkeycmp;

	if(hashtable_new_conf(&htc, &_socket_id_table) != CC_OK)
	{
        log_e("SOCKET-ID table create failed!");
	}

	if(hashtable_new_conf(&htc, &_socket_id_stats) != CC_OK)
	{
        log_e("SOCKET-ID Statistics create failed!");
	}


	msg_server_conf_t sconf;
	msg_serverconf_init(&sconf);
	sconf.on_newclient = on_serverconnect;
    sconf.on_delclient = on_serverdisconnect;
	msg_listen(port,10,&sconf);
	log_d("SERVER: listening on : id %x port %d",myid, port);
}

static int on_intoStation(uint16_t id, void* pData, int size)
{

    parkRequest_t parkReq;
    log_d("Receive %x into station request",id);
    parkReq.carId = id;   
    parkReq.type = S2C_INTO_STATION;
    memcpy(&parkReq.roadID,pData,sizeof(roadID_t));
    elog_hexdump("into pData",16,pData,5);
    Mailbox_post(parkMbox,&parkReq,BIOS_NO_WAIT);

	return 1;
}


static int on_requestID(uint16_t id, void* pData, int size)
{
    rid_t rid;
    carStatus_t carSts;

    log_d("Receive %x front-id request",id);
    rid.carId = id;
    memcpy(&rid.rfid,pData,sizeof(rfid_t)+7);
    
    /*请求ID先POST到状态处理任务，进行排队*/
    carSts.id = id;
    carSts.rfid = rid.rfid;
    carSts.dist = rid.dist;
    carSts.rpm = 0;
    carSts.mode = CAR_MODE_RUN;
    carSts.rail = rid.rail;
    carSts.carMode = rid.carMode;
    
    Mailbox_post(carStatusMbox,&carSts,BIOS_NO_WAIT);
    Mailbox_post(ridMbox,&rid,BIOS_NO_WAIT);
    return 1;
}

static int on_leaveStation(uint16_t id, void* pData, int size)
{
    parkRequest_t parkReq;
    log_d("Receive %x leave station request",id);
    parkReq.carId = id;   
    parkReq.type = S2C_LEAVE_STATION;
    memcpy(&parkReq.roadID,pData,sizeof(roadID_t));
    Mailbox_post(parkMbox,&parkReq,BIOS_NO_WAIT);
    return 1;
}

static int on_doorControl(uint16_t id, void* pData, int size)
{
    doorCtrl_t door;
    log_d("Receive %x door control request",id);
    door.carId = id;
    door.type = *(char*)pData;
    Mailbox_post(doorMbox,&door,BIOS_NO_WAIT);
    return 1;
}


static int on_carStatus(uint16_t id, void* pData, int size)
{
    carStatus_t carSts;
    statsPacket_t *stats;
    log_d("Receive %x car status",id);
    carSts.id = id;
    memcpy(&carSts.rfid,pData,sizeof(rfid_t)+9);
    if(CC_OK == hashtable_get(_socket_id_stats,id,&stats))
    {
        if(stats != NULL)
        {
            stats->packet_numsAdd++;
            //stats->packet_numsSum++;
            stats->position_current = carSts.dist;
            stats->heart_status = CAR_HEART_ACTIVED;
        }
        else
        {
            log_e("_socket_id_stats get pointer NULL");
        }
    }
    else
    {
        log_e("_socket_id_stats hash table get error");
    }
    Mailbox_post(carStatusMbox,&carSts,BIOS_NO_WAIT);
    return 1;
}

static int on_pre_adjust_request(uint16_t id, void* pData, int size)
{
    preAdjustReq_t pre_adjust_req;


    memcpy(&pre_adjust_req,pData,size);

    pre_adjust_req.car_id = id;
    pre_adjust_req.receive_moment = gettime();

    log_i("Receive %x(%.1f) preAdjust request",pre_adjust_req.car_id,pre_adjust_req.run_time);
    Mailbox_post(preAdjustMbox,&pre_adjust_req,BIOS_NO_WAIT);
    return 1;
}

static int on_serverconnect(SOCKET s, uint16_t id) 
{
    statsPacket_t *stats;
    uint32_t timestamp;
	log_i("SERVER: client connected: socket 0x%x id %x",(UINT32)s,id);

	msg_register_cb(s,S2C_INTO_STATION_CMD,on_intoStation);
	msg_register_cb(s,S2C_REQUEST_ID_CMD,on_requestID);
	msg_register_cb(s,S2C_DOOR_CONTROL_CMD,on_doorControl);
    msg_register_cb(s,S2C_LEAVE_STATION_CMD,on_leaveStation);
    msg_register_cb(s,S2C_CAR_STATUS_CMD,on_carStatus);
    msg_register_cb(s,EVENT_V2C_PRE_ADJUST_REQUEST,on_pre_adjust_request);
    /*msg_register_cb(s,S2C_ALLOT_PARK_ACK,on_allotPack);*/

    hashtable_add(_socket_id_table,id,s);

    timestamp = gettime();
    if(CC_ERR_KEY_NOT_FOUND == hashtable_get(_socket_id_stats,id,&stats))
    {
        stats = malloc(sizeof(statsPacket_t));
        if(stats != NULL)
        {
            memset(stats,0,sizeof(statsPacket_t));
            stats->connect_time[0] = timestamp;
            stats->packet_speedMin = 0xffffffff;
            hashtable_add(_socket_id_stats,id,stats);
        }
        else
        {
            log_e("stats malloc failed");
        }
    }
    else
    {
        /*统计连接次数，以及最近3次的连接时间*/
        stats->connect_nums++;
        stats->connect_time[2] = stats->connect_time[1];
        stats->connect_time[1] = stats->connect_time[0];
        stats->connect_time[0] = timestamp;
        stats->heart_status = CAR_HEART_NONE;
    }

	return 1;
}

static int on_serverdisconnect(SOCKET s, uint16_t id)
{
    SOCKET scon;
    log_i("SERVER: client disconnected: socket 0x%x id %x",(UINT32)s,id);

    if(CC_OK != hashtable_remove(_socket_id_table,id,&scon))
    {
       log_e("_socket_id_table remove failed");
    }
    return 1;
}


static SOCKET getSocket(uint16_t id)
{
    SOCKET s=NULL;
    hashtable_get(_socket_id_table,id,&s);
    return s;
}

static int msgSendByid(uint16_t id, int type, const void *pData, int len)
{
    SOCKET s=NULL;
    s = getSocket(id);
    if(s != NULL)
    {
        return msg_send(s, type, pData, len);
    }
    else
    {
        log_e("%x get invalid socket",id);
        return -1;
    }
}


/*****************************************************************************
 * 函数名称: void messageInit()
 * 函数说明: S2C消息初始化
 *  1.初始化邮箱
 *  2.初始化信用量
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
static void messageInit()
{

    carStatusMbox = Mailbox_create (sizeof (carStatus_t),S2C_MBOX_DEPTH, NULL, NULL);
    parkMbox = Mailbox_create (sizeof (parkRequest_t),S2C_MBOX_DEPTH, NULL, NULL);

    doorMbox = Mailbox_create (sizeof (doorCtrl_t),S2C_MBOX_DEPTH, NULL, NULL);
    ridMbox = Mailbox_create (sizeof (rid_t),S2C_MBOX_DEPTH, NULL, NULL);
    collisionMbox = Mailbox_create (sizeof (collisionData_t),S2C_MBOX_DEPTH, NULL, NULL);

    preAdjustMbox = Mailbox_create (sizeof (preAdjustReq_t),S2C_MBOX_DEPTH, NULL, NULL);
}
static void heart_check(const void *key)
{
    collisionData_t info;
    statsPacket_t *stats;
    hashtable_get(_socket_id_stats,key,&stats);

    if(stats != NULL)
    {
        if(stats->heart_status == CAR_HEART_FAILED)
        {
            log_e("%x heart timeout\n",key);
            if(collisionMbox != NULL)
            {
                info.carID = key;
                info.type = CONNECT_COLLISION_TYPE;
                Mailbox_post(collisionMbox,&info,BIOS_NO_WAIT);
                stats->heart_status = CAR_HEART_NONE;
            }
        }
        else if(stats->heart_status == CAR_HEART_ACTIVED)
        {
            stats->heart_status = CAR_HEART_FAILED;
        }
    }
}

static void connected_check(xdc_UArg arg)
{
//    uint8_t i,j;
//    uint8_t size;
    
    ///collisionData_t info;
    /*
     * 显示道路队列
     */
    
//    for(i=0;i<roadNums;i++)
//    {
//        size = vector_size(roadInfo[i].carQueue);
//        for(j=0;j<size;j++)
//        {
//            if(roadInfo[i].carQueue[j].heart == CAR_HEART_ACTIVED)
//                roadInfo[i].carQueue[j].heart = CAR_HEART_FAILED;
//            else
//            {
//                log_e("%x don't connect with station",roadInfo[i].carQueue[j].id);
//                if(collisionMbox != NULL)
//                {
//                    info.carID = roadInfo[i].carQueue[j].id;
//                    info.type = CONNECT_COLLISION_TYPE;
//                    Mailbox_post(collisionMbox,&info,BIOS_NO_WAIT);
//                }
//            }
//        }
//    }
//    hashtable_foreach_key(_socket_id_stats,heart_check);
    log_i("con");
}

static void taskConnectCheck(UArg arg0, UArg arg1)
{
    while(true)
    {
        Task_sleep(CONNECTED_CHECK_SLOT);
        hashtable_foreach_key(_socket_id_stats,heart_check);
    }
}
static void initTimer()
{
	Clock_Params clockParams;


	Clock_Params_init(&clockParams);
	clockParams.period = CONNECTED_CHECK_SLOT;     
	clockParams.startFlag = TRUE;//Period timer

	clockConnectHeart = Clock_create(connected_check, CONNECTED_CHECK_SLOT, &clockParams, NULL);
}

#define STATISTICS_SLOT_TIME (5000)
static void packetStatistics(const void *key)
{
    statsPacket_t *stats;
    hashtable_get(_socket_id_stats,key,&stats);

    if(stats != NULL)
    {
        if(stats->not_firstStats)
        {
            if(stats->packet_numsAdd > stats->packet_speedMax)
            {
                stats->packet_speedMax = stats->packet_numsAdd;
                stats->position_speedMax = stats->position_current;
            }

            if(stats->packet_numsAdd < stats->packet_speedMin)
            {
                stats->packet_speedMin = stats->packet_numsAdd;
                stats->position_speedMin = stats->position_current;
            }
            stats->packet_numsSum += stats->packet_numsAdd;
            stats->stats_nums++;

            log_i("%x cur(%d,%d),min(%d,%d),max(%d,%d),ave(%d),all(%d),nums(%d),con(%d),slot(%dms)",
                key,
                stats->packet_numsAdd,stats->position_current,
                stats->packet_speedMin,stats->position_speedMin,
                stats->packet_speedMax,stats->position_speedMax,
                stats->packet_numsSum / stats->stats_nums,
                stats->packet_numsSum,
                stats->stats_nums,
                stats->connect_nums,
                STATISTICS_SLOT_TIME);
        }
        else
        {
            stats->not_firstStats = 1;
        }
        
        stats->packet_numsAdd = 0;
    }

}

static void taskStatsPacket(UArg arg0, UArg arg1)
{
    while(true)
    {
        Task_sleep(STATISTICS_SLOT_TIME);
        log_i("----------------Packet Statistics-----------");
        hashtable_foreach_key(_socket_id_stats,packetStatistics);
    }
}

static void showPacketStats(const void *key)
{
    statsPacket_t *stats;
    hashtable_get(_socket_id_stats,key,&stats);
    if(stats != NULL)
    {
        sb_printf("%x cur(%d,%d),min(%d,%d),max(%d,%d),ave(%d),all(%d),nums(%d),con(%d),slot(%dms)\n",
            key,
            stats->packet_numsAdd,stats->position_current,
            stats->packet_speedMin,stats->position_speedMin,
            stats->packet_speedMax,stats->position_speedMax,
            stats->packet_numsSum / stats->stats_nums,
            stats->packet_numsSum,
            stats->stats_nums,
            stats->connect_nums,
            STATISTICS_SLOT_TIME);
    }
}

static void showConnect(const void *key)
{
    statsPacket_t *stats;
    int8_t i;
    struct tm *ptm;
    time_t now;
    hashtable_get(_socket_id_stats,key,&stats);
    if(stats != NULL)
    {
        sb_printf("%x ",key);
        for(i=0;i<3;i++)
        {
            now = stats->connect_time[i];
            if(now != 0)
            {
                ptm = localtime(&now);
                sb_printf("[%02d-%02d %02d:%02d:%02d] ", ptm->tm_mon + 1,
                        ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
            }
        }
        sb_printf("\n");
    }
}

static void psts(uint8_t argc,uint8_t **argv)
{
    if(argc == 1)
    {
        hashtable_foreach_key(_socket_id_stats,showPacketStats);
    }
    else if(argc == 2)
    {
        if(0 == strncmp("-t",argv[1],2))
        {
            hashtable_foreach_key(_socket_id_stats,showConnect);
        }
    }
}
MSH_CMD_EXPORT(psts, show packets statitics);


static void taskStartUp(UArg arg0, UArg arg1)
{

    Task_Handle task;
    Task_Params taskParams;

    uint16_t device_id;
    uint16_t port;


    stationDataInit();

    messageInit();

    ef_get_env_blob("device_id",&device_id,sizeof(device_id),NULL);
    ef_get_env_blob("device_port",&port,sizeof(device_id),NULL);
    msgServerInit(device_id,port);

//    initTimer();

    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 4096;

    taskParams.instance->name = "s2cCarStatus";
    task = Task_create((Task_FuncPtr)S2CCarStatusProcTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.instance->name = "s2cRequestID";
    task = Task_create((Task_FuncPtr)S2CRequestIDTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.instance->name = "s2cRequestPark";
    task = Task_create((Task_FuncPtr)S2CRequestParkTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.instance->name = "s2cDoorCtrl";
    task = Task_create((Task_FuncPtr)S2CDoorCtrlTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.instance->name = "s2cStopTask";
    task = Task_create((Task_FuncPtr)S2CStationStopRequestTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.instance->name = "S2CpreAdjustTask";
    task = Task_create((Task_FuncPtr)S2CpreAdjustTask, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    taskParams.instance->name = "StatsPacket";
    task = Task_create((Task_FuncPtr)taskStatsPacket, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }


    taskParams.instance->name = "connetCheck";
    task = Task_create((Task_FuncPtr)taskConnectCheck, &taskParams, NULL);
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
 * 函数名称: void S2CTaskInit()
 * 函数说明: S2C任务初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
void S2CTaskInit()
{

    Task_Handle task;
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.priority = 4;
    taskParams.stackSize = 2048;
    taskParams.instance->name = "s2cstartup";

    task = Task_create((Task_FuncPtr)taskStartUp, &taskParams, NULL);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }
}

/*****************************************************************************
 * 函数名称: static void stationDataInit()
 * 函数说明: 站台数据初始化，初始化站台区域需要使用的各类数据结构
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注:
*****************************************************************************/
static void stationDataInit()
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


int8_t findCarByID(uint16_t carID,carQueue_t *carQueue)
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

uint8_t getBSection(rfid_t rfid)
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
            log_i("Clear %x from T%d",carId,i);
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
            dist = getDistance(carSts->rfid);
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
            index = findCarByID(carSts->id,stationInfo[i].carQueue);
            if(index >= 0)
            {
                vector_erase(stationInfo[i].carQueue,index);
                log_i("%x leave T%d-P%d",carSts->id,i,stationInfo[i].carQueue[index].pid);
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
                log_i("Critical Area collision detected:%x(%d),%x(%d)",
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
    carQueue_t *carQptr;
    int32_t dist_diff;
    uint8_t areaType;
    uint8_t isBSection;

    if(carSts->carMode != AUTO_MODE)
    {
        return 0;
    }

    /*道路内无前车*/
    if(0 == getFrontCar(roadFind,carSts->id,carSts->dist,&carQptr))
    {
        return 0;
    }
    frontCar = carQptr->id;

    index_frontCar = findCarByID(frontCar,roadFind->carQueue);
    frontCarInfo = roadFind->carQueue[index_frontCar];

    dist_diff = frontCarInfo.pos - carSts->dist;

    /*站台区*/
    roadSection = getRoadSection(carSts->rfid);
    if(frontCarInfo.roadSection == roadSection && SECTION_STATION == roadSection)
    {
        if(dist_diff >= 0 && dist_diff < MIN_DISTANCE_STATION)
        {
            log_i("Station Area collision detected:%x(%d),%x(%d)",
                    carSts->id,carSts->dist,
                    frontCarInfo.id,frontCarInfo.pos);

            return STATION_COLLISION_TYPE;
        }
        else
            return 0;
    }

    /*调整区*/
    areaType = getAreaType(carSts->rfid);
    //if(frontCarInfo.areaType == areaType &&
    //        frontCarInfo.adjustNums == adjustNums &&
    //        (EREA_ADJUST_LEFT == areaType || EREA_ADJUST_LEFT == areaType))
    if(EREA_ADJUST_LEFT == areaType || EREA_ADJUST_RIGHT == areaType)
    {
        if(dist_diff >= 0 && dist_diff < MIN_DISTANCE_ADJUST)
        {
            log_i("Adjust Area collision detected:%x(%d),%x(%d)",
                    carSts->id,carSts->dist,
                    frontCarInfo.id,frontCarInfo.pos);
            return ADJUST_COLLISION_TYPE;
        }
        else
            return 0;
    }

    /*普通区*/
    isBSection = getBSection(carSts->rfid);
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
            log_i("Normal Area collision detected:%x(%d),%x(%d)",
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
            removeCarProcess(carSts.id);
            showRoadLog();
            continue;
        }

        /*
         * 更新站点车辆
         */
        if(carSts.mode == CAR_MODE_PARK)
        {
            updateStation(&carSts);
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
            log_d("%x:road ID is Wrong-%x%x%x!",carSts.id,
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
        carIsSecB = getBSection(carSts.rfid);
        distRfid = getDistance(carSts.rfid);

        /*
         * 初始化队列信息
         */
        carQ.id = carSts.id;
        carQ.mode = carSts.mode;
        carQ.rpm = carSts.rpm;
        carQ.pos = 0;           /*默认值*/
        carQ.pid = 0;           /*默认值*/
        carQ.areaType = getAreaType(carSts.rfid);
        carQ.roadSection = getRoadSection(carSts.rfid);
        carQ.adjustNums = getAdjustZoneNums(carSts.rfid);
        carQ.isBSection = carIsSecB;
        carQ.carMode = carSts.carMode;
        carQ.rail = carSts.rail;
        carQ.heart = CAR_HEART_ACTIVED;
        carQ.rfid = carSts.rfid;
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
                log_i("%x is out of Seperate Zone");
                continue;
            }


            carSepIsInserting = 1;
        }

        /*
         * 碰撞风险检测
         */
#if 0
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
#endif

        /*
         * --------------------- 路段队列处理   ------------------------------
         * 1.对应轨道已存在该车辆，则更新车辆信息；
         * 2.对应轨道不存在该车辆，则插入该车辆；
         * 3.其它轨道上存在该车辆，则从队列中删除该车辆；
         */
        carIsInserting = 1;

        for(i=0;i<roadNums;i++)
        {
            index = findCarByID(carSts.id,roadInfo[i].carQueue);
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
                        log_i("Status:%x(%d,%d) del %d from %x%x%x %04x",carSts.id,
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
                log_i("Status:%x(%d,%d) insert %d to %x%x%x %04x",carSts.id,
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
                log_i("Status:%x(%d,%d) insert %d to %x%x%x %04x",carSts.id,
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
                index = findCarByID(carSts.id,adjustZone[i].carQueue);
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
            ajustNum = getAdjustZoneNums(carSts.rfid);
            if(ajustNum < S2C_ADJ_NUMS)
            {
                if(adjustZone[ajustNum].start <= carSts.dist && carSts.dist <= adjustZone[ajustNum].end)
                {
                    index = findCarByID(carSts.id,adjustZone[ajustNum].carQueue);
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
                    log_i("Manual Mode:%x Adjust scale error",carSts.id);
                }
            }
            else
            {
                log_i("Manual Mode:Adjust Number error",carSts.id);
            }
        }

        if(isShowRoad == 1)
            showRoadLog();
    }
}

static uint32_t pre_adjust_delay = 15;
SHELL_EXPORT_VAR_INT(adj_delay,pre_adjust_delay,pre-Adjust Delay(defalut 15s));

static void S2CpreAdjustTask(UArg arg0, UArg arg1)
{
    preAdjustInfo_t preAdjust_info;
    uint32_t goal_moment = 0;
    uint32_t last_except_delay_moment = 0;
    uint32_t a,b,c,d;
    while(1)
    {
        Mailbox_pend(preAdjustMbox,&preAdjust_info.preAdjust_req,BIOS_WAIT_FOREVER);
        a = preAdjust_info.preAdjust_ack.except_moment;
        b = pre_adjust_delay;
        last_except_delay_moment = a+b;
        //last_except_delay_moment = (float)preAdjust_info.preAdjust_ack.except_moment + (float)pre_adjust_delay;
        c = preAdjust_info.preAdjust_req.receive_moment;
        d = (uint32_t)preAdjust_info.preAdjust_req.run_time;
        goal_moment = c + d;
        //goal_moment = (float)preAdjust_info.preAdjust_req.receive_moment + preAdjust_info.preAdjust_req.run_time;
        log_w("last except moment(%d,%d,%d)",a,
                                                    b,
                                                    last_except_delay_moment);
        log_w("current run moment(%d,%d,%d)",c,
                                            d,
                                            goal_moment);
        if(goal_moment >= last_except_delay_moment)
        {
            /*
             * 到达时刻 > 预期延迟时间
             */
            preAdjust_info.preAdjust_ack.except_moment = goal_moment;
            preAdjust_info.preAdjust_ack.except_time = preAdjust_info.preAdjust_req.run_time;
        }
        else
        {

            preAdjust_info.preAdjust_ack.except_moment = last_except_delay_moment;
            preAdjust_info.preAdjust_ack.except_time = last_except_delay_moment - preAdjust_info.preAdjust_req.receive_moment;
        }

        msgSendByid(preAdjust_info.preAdjust_req.car_id,
                EVENT_V2C_PRE_ADJUST_ACK,
                &preAdjust_info.preAdjust_ack.except_time,
                sizeof(float));

        log_w("preAdjust ack %x run time(%.1f),except time:(%.1f)",
                preAdjust_info.preAdjust_req.car_id,
                preAdjust_info.preAdjust_req.run_time,
                preAdjust_info.preAdjust_ack.except_time
                );
    }
}

static uint8_t getFrontCar(roadInformation_t *road,uint16_t carID,uint32_t dist,carQueue_t *frontCar)
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
        index = findCarByID(carID,road->carQueue);


        if(index < 0)
        {

            if(road->isRing == 0)
                index = S2CFindCarByPosition(road->carQueue,dist);
            else
                index = S2CRingFindCarByPosition(road->carQueue,dist);
            log_i("find car by pos %x(%d)",dist,index);
        }
        else
        {
            log_i("find car by id %x(%d)",road->carQueue[index].id,index);
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
                frontCar = &(road->carQueue[size-1]);
                frontCarDist = road->carQueue[size-1].pos;
                found = 1;
                log_i("front car %x(%d)",road->carQueue[size-1].id,size-1);
            }

        }
        else
        {
            frontCar = &(road->carQueue[index-1]);
            frontCarDist = road->carQueue[index-1].pos;
            found = 1;
            log_i("front car %x(%d)",road->carQueue[index-1].id,index-1);
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
            log_i("front car is too far");
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
    carQueue_t *carQptr;
    uint8_t *str;
    while(1)
    {
        Mailbox_pend(ridMbox,&rid,BIOS_WAIT_FOREVER);

        carQ.id = rid.carId;
        carQ.mode = 0;
        carQ.rpm = 0;
        carQ.rail = rid.rail;
        carQ.heart = CAR_HEART_ACTIVED;
        carQ.rfid = rid.rfid;
        carQ.pos = rid.dist;
        memcpy(&carQ.roadID,&rid.rfid.byte[1],sizeof(roadID_t));
        /*
         * 确定车辆所属路线
         */
        areaType = getAreaType(rid.rfid);
        distRfid = getDistance(rid.rfid);
        carIsSecB = getBSection(rid.rfid);

        memcpy(&roadID,&rid.rfid.byte[1],sizeof(roadID_t));
        if(areaType == EREA_SEPERATE && rid.rail == RIGHT_RAIL)
        {
            roadID = S2CFindSeparateRoad(roadID,distRfid);
        }

        if(areaType == EREA_SEPERATE)
            str = "Sepr:";
        else if(areaType == EREA_ADJUST_RIGHT)
            str = "RAdj:";
        else if(areaType == EREA_ADJUST_LEFT)
            str = "LAdj:";
        else
            str = "Norm:";

        log_i("%s%x(%d,%d,%d)Request ID.<%x%x%x %04x>",str,rid.carId,
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
            log_w("Warn:%x EPC out of range",rid.carId);
            frontCar.state = 0;
            frontCar.carID[0] = 0;
            frontCar.carID[1] = 0;
            log_i("ACK:%x -> none",rid.carId);
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
                areaType = getAreaType(rid.rfid);


                adjustZoneNums = getAdjustZoneNums(rid.rfid);


                if(adjustZoneNums > S2C_ADJ_NUMS)
                {
                    log_w("Adjust Number Error!");
                    continue;
                }
                else
                {
                    adjustZonePtr = getAdjustZone(adjustZoneNums);
                    if(adjustZonePtr->start > rid.dist || adjustZonePtr->end < rid.dist)
                    {
                        log_w("Adjust Scale Error!");
                        continue;
                    }
                }

                index = findCarByID(rid.carId,adjustZonePtr->carQueue);
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
                //adjustZoneNums = getAdjustZoneNums(rid.rfid);
                //adjustZonePtr = getAdjustZone(adjustZoneNums);
                frontCar.state=0;
                size = vector_size(adjustZonePtr->carQueue);

                if(size > 1)
                {
                    /*
                     * 调整区有前车
                     * ps:车辆在调整区申请时，已经处于调整区队列的队尾
                     */
                    index = findCarByID(rid.carId,adjustZonePtr->carQueue);
                    if(index < 0)
                    {
                        frontCar.state = 0;
                        log_w("Adjust Queue Error!");
                    }
                    else
                    {
                        if(index == 0)
                        {
                            frontCar.state = 0;
                            log_i("no front car in adjust zone");
                        }
                        else
                        {
                            frontCar.state = 1;
                            
                            frontCar.rfid[0] = adjustZonePtr->carQueue[index-1].rfid;
                            frontCar.dist[0] = adjustZonePtr->carQueue[index-1].pos;
                            frontCar.carID[0] = adjustZonePtr->carQueue[index-1].id;
                            frontCar.carID[1] = 0;
                            log_i("front car %x(%d)in adjust zone",adjustZonePtr->carQueue[index-1].id,index-1);

                            /*查找另外一条轨道上的前车*/
                            for(i=(index-2);i>=0;i--)
                            {
                                //if(adjustZonePtr->carQueue[index-1].rail != adjustZonePtr->carQueue[i].rail)
                                if(memcmp(&adjustZonePtr->carQueue[index-1].roadID,&adjustZonePtr->carQueue[i].roadID,sizeof(roadID_t)))
                                {
                                    frontCar.rfid[1] = adjustZonePtr->carQueue[i].rfid;
                                    frontCar.dist[1] = adjustZonePtr->carQueue[i].pos;
                                    frontCar.carID[1] = adjustZonePtr->carQueue[i].id;
                                    log_i("front car %x(%d)in other road",adjustZonePtr->carQueue[i].id,i);
                                    break;
                                }
                            }
                        }
                    }

                }
                if(frontCar.state == 0)
                {
                    /*
                     * 调整区无前车，返回主道上的车辆
                     */
                    log_i("find front car from main road");
                    for(i=0;i<roadNums;i++)
                    {
                        if(0 == memcmp(&adjustZonePtr->leftRoadID,&roadInfo[i].roadID,sizeof(roadID_t)))
                        {
                            roadAdjust = &roadInfo[i];
                            break;
                        }
                    }
                    frontCar.state = getFrontCar(roadAdjust,rid.carId,dist,&carQptr);
                    frontCar.rfid[0] = carQptr->rfid;
                    frontCar.dist[0] = carQptr->pos;
                    frontCar.carID[0] = carQptr->id;
                    frontCar.carID[1] = 0;
                    log_i("front car %x from main road",carQptr->id);
                }
            }
            else
            {
                frontCar.state = getFrontCar(roadFind,rid.carId,dist,&carQptr);
                frontCar.rfid[0] = carQptr->rfid;
                frontCar.dist[0] = carQptr->pos;
                frontCar.carID[0] = carQptr->id;
                frontCar.carID[1] = 0;
                log_i("front car %x",carQptr->id);
            }

            if(frontCar.state == 0)
            {
                /*
                 * 无前车
                 */
                log_i("ACK:%x -> none",rid.carId);
            }
            else
            {
                log_i("ACK:%x -> %x,%x",rid.carId,frontCar.carID[0],frontCar.carID[1]);

            }
            showRoadLog();

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

        
        msgSendByid(rid.carId,S2C_REQUEST_ID_ACK,&frontCar,sizeof(frontCar_t));
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
    stationInformation_t *stationFind;
    int8_t index;
    uint8_t size;
    carQueue_t carQ;
    uint8_t state;
    uint8_t tN;
    uint8_t stationFlag = 0;
    intoStationAck_t intoStationAck;
    while(1)
    {
        Mailbox_pend(parkMbox,&parkReq,BIOS_WAIT_FOREVER);

        /*
         * 判断报文类型
         */
        if(parkReq.type != S2C_INTO_STATION)
            continue;

        log_i("Park:%x request park",parkReq.carId);
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
            log_w("%x request park out of range:%x%x%x",parkReq.carId,
                    parkReq.roadID.byte[0],
                    parkReq.roadID.byte[1],
                    parkReq.roadID.byte[2]);
            showRoadLog();
            continue;
        }

        carQ.id = parkReq.carId;
        carQ.mode = 0;
        carQ.rpm = 0;
        carQ.pos = 0;
        carQ.heart = CAR_HEART_ACTIVED;
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

            index = findCarByID(parkReq.carId,stationFind->carQueue);

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

        intoStationAck.status = 1;
        intoStationAck.rfid[0] = 0;
        memcpy(&intoStationAck.rfid[1],&stationFind->roadID,sizeof(roadID_t)+2);
        intoStationAck.rfid[8] = (stationFind->park[carQ.pid].trigger >> 16) & 0xff;
        intoStationAck.rfid[9] = (stationFind->park[carQ.pid].trigger >> 8) & 0xff;
        intoStationAck.rfid[10] = (stationFind->park[carQ.pid].trigger) & 0xff;
        intoStationAck.rfid[11] = 0;
        
        msgSendByid(parkReq.carId,S2C_INTO_STATION_ACK,&intoStationAck,sizeof(intoStationAck_t));

        log_i("Park:%x -> T%d.P%d-%x",carQ.id,tN,carQ.pid,stationFind->park[carQ.pid].trigger);

        showStationLog();
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

    doorStatus_t dSts;
    doorStatus_t * doorNode = 0;
    uint8_t state;
    int8_t index;
    stationInformation_t *stationFind;
    doorAck_t doorAck;
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
                index = findCarByID(door.carId,stationInfo[i].carQueue);
                if(index >= 0)
                {
                    stationFind = &stationInfo[i];
                    dSts.tid = i;
                    break;
                }
            }

            if(stationFind == 0)
            {
                log_e("Car%x request door out of station",door.carId);
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

                log_i("Open door of station-T%d-P%d",dSts.tid,dSts.pid);
            }
            else if(door.type == S2C_CLOSE_DOOR)
            {
                /*
                 *  1.关门
                 *  2.等待关门结束
                 *  3.发送关门状态
                 */
                log_i("Close door of station-T%d-P%d",dSts.tid,dSts.pid);
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
                
                doorAck.operation = doorNode[i].type;
                doorAck.status = 1;
                msgSendByid(doorNode[i].carId,S2C_DOOR_CONTROL_ACK,&doorAck,sizeof(doorAck_t));

                vector_erase(doorNode,i);
            }
        }
#endif
    }
}


static void S2CStationStopRequestTask(UArg arg0, UArg arg1)
{
    collisionData_t collisionInfo;
    uint8_t i,j;
    uint8_t size;
    int8_t index;
    uint8_t retryNums;
    uint8_t isEnd;
    stopRequest_t stopRequest;
    while(1)
    {
        Mailbox_pend(collisionMbox,&collisionInfo,BIOS_WAIT_FOREVER);
        retryNums = 0;
        sb_printf("collision stop\n");
        do
        {
            isEnd = 1;
            if(collisionInfo.type == STATION_COLLISION_TYPE)
            {
                for(i=0;i<roadNums;i++)
                {
                    index = findCarByID(collisionInfo.carID,roadInfo[i].carQueue);
                    if(index > 0)
                    {
                        if(roadInfo[i].carQueue[index].carMode != STOP_MODE)
                        {
                            log_i("Collision Stop %x,type %d",collisionInfo.carID,collisionInfo.type);
                            
                            stopRequest.collision = collisionInfo.type;
                            msgSendByid(collisionInfo.carID,S2C_REQUEST_STOP,&stopRequest,sizeof(stopRequest_t));

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
                sb_printf("collision stop retry %d\n",retryNums);
                for(i=0;i<roadNums;i++)
                {
                    size = vector_size(roadInfo[i].carQueue);
                    for(j=0;j<size;j++)
                    {
                        if(roadInfo[i].carQueue[j].carMode != STOP_MODE)
                        {
                            log_i("Collision Stop %x,type %d",roadInfo[i].carQueue[j].id,collisionInfo.type);
//                            sb_printf("Collision Stop %x,type %d\n",roadInfo[i].carQueue[j].id,collisionInfo.type);

                            stopRequest.collision = collisionInfo.type;
                            msgSendByid(roadInfo[i].carQueue[j].id,S2C_REQUEST_STOP,&stopRequest,sizeof(stopRequest_t));
                            isEnd = 0;
                            Task_sleep(50);
                        }
                    }
                }
                retryNums ++;
            }
            Task_sleep(500);
        }while(retryNums < 5 && isEnd == 0);
    }
}


void showStationLog()
{
    uint8_t i,j;
    uint8_t size;
    
    char str[32];

    strcpy(log_buf,"\r\ncurrent STATION status\r\n");

    /*
     * 显示站台队列
     */
    for(i=0;i<termNums;i++)
    {
        
        sprintf(str,"T%d:",i);
        strcat(log_buf,str);

        for(j=0;j<stationInfo[i].parkNums;j++)
        {
            sprintf(str,"%2x,",stationInfo[i].carStation[stationInfo[i].parkNums-1-j].id);
            strcat(log_buf,str);
        }
        
        sprintf(str,"    Q%d:",i);
        strcat(log_buf,str);

        size = vector_size(stationInfo[i].carQueue);
        for(j=0;j<size;j++)
        {
            sprintf(str,"%2x,",stationInfo[i].carQueue[size-1-j].id);
            strcat(log_buf,str);
        }
        strcat(log_buf,"\r\n");
    }

    log_buf[255] = 0;
    log_i("%s",log_buf);
}

void showRoadLog()
{
#if 1
    uint8_t i,j;
    uint8_t size;
    char str[32];
    /*
     * 显示道路队列
     */
    
    strcpy(log_buf,"\r\ncurrent ROAD status\r\n");
    for(i=0;i<roadNums;i++)
    {
        sprintf(str,"R%x%x%x:",roadInfo[i].roadID.byte[0],
                roadInfo[i].roadID.byte[1],
                roadInfo[i].roadID.byte[2]);
        strcat(log_buf,str);

        size = vector_size(roadInfo[i].carQueue);
        for(j=0;j<size;j++)
        {
            sprintf(str,"->%x(%d)",roadInfo[i].carQueue[size-1-j].id,
                    roadInfo[i].carQueue[size-1-j].pos);
            strcat(log_buf,str);
        }
        strcat(log_buf,"\r\n");
    }

    /*
     * 显示调整区队列
     */
    for(i=0;i<adjNums;i++)
    {
        sprintf(str,"Adj%d:",adjustZone[i].adjustNums);
        strcat(log_buf,str);
        size = vector_size(adjustZone[i].carQueue);
        for(j=0;j<size;j++)
        {
            sprintf(str,"->%x(%d)",adjustZone[i].carQueue[size-1-j].id,
                    adjustZone[i].carQueue[size-1-j].pos);
            strcat(log_buf,str);
        }
        strcat(log_buf,"\r\n");
    }
    log_buf[255] = 0;
    log_i("%s",log_buf);
#endif
}


void S2CSetStationStatus(uint8_t state)
{
    stationStatus = state;
}

void S2CSetCarNums(uint8_t nums)
{
    stationCarNums = nums;
}

static uint32_t getDistance(rfid_t rfid)
{
    uint32_t dist;
    dist = (rfid.byte[8]<<16) +
           (rfid.byte[9]<<8) +
            rfid.byte[10];
    return dist;
}

static uint8_t getAreaType(rfid_t rfid)
{
    uint8_t Area;
    Area = rfid.byte[7] >> 6;

    return Area;
}

static uint8_t getAdjustZoneNums(rfid_t rfid)
{
    uint8_t nums;
    nums = (rfid.byte[6] & 0x7f) >> 2;
    return nums;
}

static uint8_t getRoadSection(rfid_t rfid)
{
    uint8_t road_section;
    road_section = (rfid.byte[6] & 0x03);
    return road_section;
}

static adjustZone_t* getAdjustZone(uint8_t nums)
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

static void removeCarProcess(uint16_t carID)
{
    uint8_t i;
    int8_t index;
    for(i=0;i<roadNums;i++)
    {
        index = findCarByID(carID,roadInfo[i].carQueue);
        if(index >= 0)
        {
            /*
             * 找到车辆
             */
            vector_erase(roadInfo[i].carQueue,index);
            log_i("%x remove %d from %x%x%x",carID,index,
                    roadInfo[i].roadID.byte[0],
                    roadInfo[i].roadID.byte[1],
                    roadInfo[i].roadID.byte[2]);

        }
    }

    for(i=0;i<adjNums;i++)
    {
        index = findCarByID(carID,adjustZone[i].carQueue);
        if(index >= 0)
        {
            /*
             * 找到车辆
             */
            vector_erase(adjustZone[i].carQueue,index);
        }
    }
}

static void updateStation(carStatus_t *carSts)
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
                distRfid = getDistance(carSts->rfid);

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

void webShowlog(char * htmlbuf,size_t buflen)
{
    int len = 0;
    int i,j,size;
    for(i=0;i<roadNums;i++)
    {
        len += snprintf(htmlbuf+len,buflen-len-1,"<tr><td>R%x%x%x:</td><td>",
                roadInfo[i].roadID.byte[0],
                roadInfo[i].roadID.byte[1],
                roadInfo[i].roadID.byte[2]);

        size = vector_size(roadInfo[i].carQueue);
        if(size == 0)
        {
            len += snprintf(htmlbuf+len,buflen-len-1,"no car");
        }
        else
        {
            for(j=0;j<size;j++)
            {
                if(len < (buflen-1))
                {
                    len += snprintf(htmlbuf+len,buflen-len-1,"%x(%d) ",
                            roadInfo[i].carQueue[size-1-j].id,
                            roadInfo[i].carQueue[size-1-j].pos);
                }
            }
        }

        if(len < (buflen-1))
        {
            len += snprintf(htmlbuf+len,buflen-len-1,"</td></tr>\r\n");
        }
    }
}
/*Shell command*/
static int showlog(uint8_t argc,uint8_t **argv)
{
    uint8_t i,j;
    uint8_t size;
    /*
     * 显示站台队列
     */
    for(i=0;i<termNums;i++)
    {
        sb_printf("\r\nT%d:",i);

        for(j=0;j<stationInfo[i].parkNums;j++)
        {
            sb_printf("%2x ",stationInfo[i].carStation[stationInfo[i].parkNums-1-j].id);
        }
        
        sb_printf("   Q%d:",i);
        size = vector_size(stationInfo[i].carQueue);
        for(j=0;j<size;j++)
            sb_printf("%2x ",stationInfo[i].carQueue[size-1-j].id);
    }
    /*showRoadLog();*/
    /*
     * 显示道路队列
     */
    for(i=0;i<roadNums;i++)
    {
        sb_printf("\r\nR%x%x%x:",roadInfo[i].roadID.byte[0],
                roadInfo[i].roadID.byte[1],
                roadInfo[i].roadID.byte[2]);
        size = vector_size(roadInfo[i].carQueue);
        for(j=0;j<size;j++)
            sb_printf("->%x(%d)",roadInfo[i].carQueue[size-1-j].id,
                    roadInfo[i].carQueue[size-1-j].pos);
    }

    /*
     * 显示调整区队列
     */
    for(i=0;i<adjNums;i++)
    {
        sb_printf("\r\nAdj%d:",adjustZone[i].adjustNums);
        size = vector_size(adjustZone[i].carQueue);
        for(j=0;j<size;j++)
            sb_printf("->%x(%d)",adjustZone[i].carQueue[size-1-j].id,
                    adjustZone[i].carQueue[size-1-j].pos);
    }
    return 0;
}
MSH_CMD_EXPORT(showlog,show station and road status);


static int delcar(uint8_t argc,uint8_t **argv)
{
    uint16_t id;
    SOCKET s;
    statsPacket_t * stats;
    if(argc < 2)
    {
        sb_puts("Please input car id\r\n",-1);
    }
    else
    {
        id = strtoul(argv[1],NULL,16);

        if(CC_OK == hashtable_remove(_socket_id_stats,id,&stats))
        {
            if(stats != NULL)
            {
                free(stats);
            }
        }
        
        if(CC_OK == hashtable_remove(_socket_id_table,id,&s))
        {
            if(s != NULL)
            {
                fdOpenSession(Task_self());
                /*release SOCKET */
                fdClose(s);
            }
            sb_printf("remove %x from station\r\n",id);
        }
        else
        {
            sb_printf("can not find %x\r\n",id);
        }
        removeCarProcess(id);
    }

    return 0;
}
MSH_CMD_EXPORT(delcar, remove car from station);

#endif
