
#ifndef __CAN_MODULE__
#define __CAN_MODULE__

#ifdef __cplusplus
extern "C"{
#endif
#include <xdc/runtime/Log.h>
#include "sja_common.h"
#include "soc_C6748.h"

#define ZMD_SJA1000_MODULE 1

/*define*/
#define NELEMENTS(array)		/* number of elements in an array */ \
		(sizeof (array) / sizeof ((array) [0]))
/*调试信息控制*/
#define CAN_DEBUG_LOG(level,fmt,arg0,arg1,arg2,arg3,arg4,arg5) do{\
		if (level <= DBG_LEVEL) { \
			logMsg(0,"\rcan:%d\n",arg0); \
			logMsg(1,"\r%s(%d):"fmt"\n",__FUNCTION__,__LINE__,arg1,arg2,arg3,arg4,arg5);  \
		}\
	}while(0)
/*目前支持的CAN波特率*/
#define	BAUDTATE_20K        (0)            /*0 ByteRate_20k*/  /*Not Supoort*/           
#define	BAUDTATE_40K        (1)            /*1 ByteRate_40k*/           
#define	BAUDTATE_50K        (2)            /*2 ByteRate_50k*/           
#define	BAUDTATE_80K        (3)            /*3 ByteRate_80k*/           
#define	BAUDTATE_100K       (4)            /*4 ByteRate_100k*/          
#define	BAUDTATE_125K       (5)            /*5 ByteRate_125k*/          
#define	BAUDTATE_200K       (6)            /*6 ByteRate_200k*/          
#define	BAUDTATE_250K       (7)            /*7 ByteRate_250k*/          
#define	BAUDTATE_400K       (8)            /*8 ByteRate_400k*/          
#define	BAUDTATE_500K       (9)            /*9 ByteRate_500k*/          
#define	BAUDTATE_666K       (10)           /*a ByteRate_666k*/          
#define	BAUDTATE_800K       (11)           /*b ByteRate_800k*/          
#define	BAUDTATE_1000K      (12)           /*c ByteRate_1000k:NBT=6TQ*/ 
/*CAN模块IO控制*/
#define  CAN_BAUD_SET		(0)/*波特率设置*/
#define  CAN_BAUD_GET		(1)/*获取波特率*/
#define  CAN_ACCCODE		(2)/*滤波设置*/
#define  CAN_ACCMASK		(3)/*滤波掩码设置*/
#define  CAN_FILER			(4)/*滤波方式设置*/
#define	 CAN_MODE			(5)/*CAN(sja1000独有)模式选择(basicCAN peliCAN)*/
#define  CAN_FRAM_MODE		(6)/*CAN帧模式(标准帧 扩展帧)*/
#define  CAN_DEVINFO_GET	(7)/*can模块信息打印*/
#define  CAN_DEBUG			(8)/*can模块信息打印级别控制*/
#define  CAN_RESET			(9)/*CAN芯片硬件复位*/
#define  CAN_BUS_STATUS		(10)/*can总线状态*/
/*CAN滤波方式宏定义*/
#define FILTER_DUAL          (0<<16)        /*双滤波*/
#define FILTER_SINGLE        (1<<16)        /*单滤波*/
/*CAN工作方式宏定义。用于open函数中的flags参数配置*/
#define OPERATION_NORMAL     0        /*正常模式*/
#define OPERATION_RDONLY     1        /*只听模式*/
#define OPERATION_LOOPBACK   2        /*自测模式*/
/*CAN帧格式定义*/
#define CAN_FRAME_TYPE_STD	 (0)	/*标准帧*/
#define CAN_FRAME_TYPE_EXT	 (1)	/*扩展帧*/
#define CAN_FRAME_TYPE_DATA	 (0)	/*数据帧*/
#define CAN_FRAME_TYPE_RTR	 (1)	/*远程帧*/
/*CAN中断类型定义*/
#define RIE			(1<<0) 			  /*接收中断*/
#define TIE			(1<<1)			  /*发送中断*/	
/*设备状态*/
#define CAN_DEV_OPENED			(0xffff)
#define CAN_DEV_CLOSED			(0)

#define CAN_INT_STATUS_ADDR (SOC_EMIFA_CS2_ADDR + (0x21<<1))
#define CAN_RESET_ADDR (SOC_EMIFA_CS2_ADDR + (0x23<<1))
#define CAN_INT_ENABLE_ADDR (SOC_EMIFA_CS2_ADDR + (0x26<<1))
#define CAN_INT_MASK_ADDR (SOC_EMIFA_CS2_ADDR + (0x27<<1))

/* CAN缓冲的最大深度 */
#define CAN_BUFFER_MAX_DEPTH 32


#define canHardIntEnable() \
    (*(volatile UINT16 *) (CAN_INT_ENABLE_ADDR) = 1)
    
#define canHardIntDisable() \
    (*(volatile UINT16 *) (CAN_INT_ENABLE_ADDR) = 0)

#define canHardIntMaskAll() \
    (*(volatile UINT16 *) (CAN_INT_MASK_ADDR) = 0xff)
    
#define canHardIntUnMaskAll() \
    (*(volatile UINT16 *) (CAN_INT_MASK_ADDR) = 0)

#define canGetHardIntStatus() \
    (~(*(volatile UINT16 *) (CAN_INT_STATUS_ADDR)))


/*typedef*/
#pragma pack(1)/*考虑字节对齐操作*/
/*can诊断项定义*/
typedef struct _can_diagnose
{
	UINT32   nIntCount;	/*产生的中断次数*/
	UINT32   nIntRecvCount;	/*产生的接收中断次数*/
	UINT32   nIntSendCount;	/*产生的发送中断次数*/
	UINT32   nWriteCount;	/*应用调用写的次数*/
	UINT32   nReadCount;   /*应用调用读的次数*/
	UINT32   nBEICount; /*总线错误中断次数*/
	UINT32   nEICount;	/*错误报警中断次数*/
	UINT32   nDOICount;	/*数据溢出中断次数*/
	UINT32   nEPICount;	/*错误消极中断次数*/
	UINT32   nALIICount;	/*仲裁丢失中断次数*/
	UINT8    nALErrValue;  /*最后一次仲裁丢失的错误代码值*/
	UINT8    nErrValue;  /*最后一次的错误代码值*/	
	UINT8    nTxErrValue;  /*发送错误计数*/
	UINT8    nRxErrValue;  /*接收错误计数*/		
} CAN_DIAGNOSE;
/*can数据结构定义*/
typedef struct _can_data_obj
{
	UINT32  ID;/*报文ID*/
	UINT8 SendType;/*发送帧类型，=0时为正常发送，=1时为单次发送，=2时为自发自收，
	                        =3时为单次自发自收，只在此帧为发送帧时有意*/
	UINT8 RemoteFlag;/*是否是远程帧；1/远程帧 0/非远程帧*/
	UINT8 ExternFlag;/*是否是扩展帧,1/扩展帧 0/标准帧*/
	UINT8 DataLen;/*数据长度(<=8)，即Data 的长度*/
	INT8  Data[8];/*报文的数据；*/
}CAN_DATA_OBJ;

typedef struct _can_buffer_obj
{
    CAN_DATA_OBJ            LoopBuffer[CAN_BUFFER_MAX_DEPTH];
    UINT8                     WritePoint;
    UINT8                      ReadPoint;
}CAN_BUFFER_OBJ;

/*
 @var typedef HW_INT_PARAMS CAN_INT_TABLE
 @brief struct
 @ingroup dspchip_can_api_functions
 */
 typedef struct HW_INT_PARAMS 
{
	UINT32 eventId;/*中断事件号*/
	UINT32 intTxRx;/*CAN接收发送中断使能*/
	ISRPFUNC irqCall;/*中断回调*/
	INT32  arg;/*中断回调函数参数*/
}CAN_INT_TABLE;
/*
 @var typedef HW_CAN_PARAMS CAN_PARAMS_TABLE
 @brief struct
 @ingroup dspchip_can_api_functions
 */
typedef struct HW_CAN_PARAMS 
{
	UINT32 addr_index;/*SJA1000 EMIF总线基地址 ECAN表示模块内索引*/
	UINT32 width_gpioArray;/*SJA1000 EMIF总线位宽 ECAN使用那组GPIO*/
	UINT32 frameMode;/*CAN控制器的帧模式 对于sja1000来说帧模块还包括滤波方式 高16位存滤波方式 低16位存帧模式*/
	UINT32 acr;	/*验收屏蔽寄存器*/
	UINT32 amr;	/*验收掩码寄存器*/
	UINT32 rate;/*波特率设置*/
}CAN_PARAMS_TABLE;
/*
 @var typedef HW_CAN_PARAMS CAN_PARAMS_TABLE
 @brief struct
 @ingroup dspchip_can_api_functions
 */
typedef struct HW_CAN_PARAMS_INT 
{
	CAN_PARAMS_TABLE can_params_table;/*CAN参数表*/
	CAN_INT_TABLE    can_int_cfg_table;/*CAN中断配置表*/
	CAN_DIAGNOSE	can_diagnose_table;/*CAN诊断表*/
}CAN_PARAMS_INT_TABLE;
/*
 @var typedef HW_CAN_CFG CAN_CFG_TABLE
 @brief struct
 @ingroup dspchip_can_api_functions
 */
typedef struct HW_CAN_CFG 
{
	UINT8*      	 		hwName;			/*设备名*/
	UINT8 					devsNum;		/*设备序列号*/
	CAN_PARAMS_INT_TABLE 	hwCfg;			/*设备资源信息*/
    CAN_BUFFER_OBJ          lpBuf;          /*设备Loop-Buffer*/
}CAN_CFG_TABLE;
/*
 @var typedef INT32 (*PHWCANOPEN)(UINT8 devNum)
 @brief function
 @ingroup dspchip_can_api_functions
 */
typedef INT32 (*PHWCANOPEN)(void *canObj);	

/*
 @var typedef INT32 (*PHWCANWRITE)(UINT8 devNum,unsigned short devId,unsigned short devOffset,UINT8 * dataSrc,UINT32 devDataLen)
 @brief function
 @ingroup dspchip_can_api_functions
 */
typedef INT32 (*PHWCANWRITE)(void *canObj,void * canData);

/*
 @var typedef INT32 (*PHWCANREAD)(UINT8 devNum,unsigned short devId,unsigned short devOffset,UINT8 * dataDst,UINT32 devDataLen)
 @brief function
 @ingroup dspchip_can_api_functions
 */
typedef INT32 (*PHWCANREAD)(void *canObj,void * canData);

/*
 @var typedef INT32 (*PHWCANCLOSE)(UINT8 devNum)
 @brief function
 @ingroup dspchip_can_api_functions
 */
typedef INT32 (*PHWCANCLOSE)(void *canObj);

/*
 @var typedef INT32 (*PHWCANCTRL)(UINT8 devNum,UINT32 devCtrlOption,void* devCtrlData)
 @brief function
 @ingroup dspchip_can_api_functions
 */
typedef INT32 (*PHWCANCTRL)(void *canObj,UINT8 funcNo,UINT32 *arg);
/*
 @struct CAN_FUNC
 @brief can底层操作API结构体.
 @ingroup dspchip_can_api_functions
*/

typedef void (*PHWCANISR)(INT32 arg);



typedef struct CAN_FUNC
{
	UINT8*       	hwName;			/*设备名*/
	PHWCANOPEN  	hwCanOpen;		/**< chip CAN open*/
	PHWCANWRITE 	hwCanWrite;		/**< chip CAN write*/
	PHWCANREAD  	hwCanRead;		/**< chip CAN read*/
	PHWCANCLOSE 	hwCanClose;		/**< chip CAN close*/
	PHWCANCTRL  	hwCanCtl;		/**< chip CAN ctrl*/
    PHWCANISR       hwcanisr;       /**< chip CAN ISR*/
}CAN_FUNC_TABLE;
/*
 @struct CAN_DEV
 @brief can底层操作API结构体.
 @ingroup dspchip_can_api_functions
*/
typedef struct CAN_DEV
{
	UINT32 devStatus;		/*设备状态*/
	UINT8	devsNum;	/*设备序列号*/
	UINT8   hwName[20];	/*设备名 NOTE:设备名长度不能超过20*/
	UINT32 drvIndex;		/*设备驱动索引*/
	UINT32 paramIndex;	/*设备资源索引*/
}CAN_DEV_TABLE;
#pragma pack()
/*CAN API接口定义*/
extern INT32 canTableInit(void);
extern INT32 canOpen(UINT8 devsNum,ISRPFUNC canIsrCall,INT32 arg);
extern INT32 canClose(UINT8 devsNum);
extern INT32 canWrite(UINT8 devsNum,CAN_DATA_OBJ * canData);
extern INT32 canRead(UINT8 devsNum,CAN_DATA_OBJ * canData);
extern INT32 canIoCtl(UINT8 devsNum,UINT8 funcNo,UINT32 *arg);
void canIsr(INT32 arg);
static canHardWareReset(UINT8 devsNum);
void canInitBuffer(UINT8 devsNum);
UINT8 canBufferIsEmpty(UINT8 devsNum);
CAN_DATA_OBJ* canPopBuffer(UINT8 devsNum);
CAN_DATA_OBJ * canPushBuffer(UINT8 devsNum);
void canHardIntMask(UINT8 devsNum);
void canHardIntUnmask(UINT8 devsNum);


#ifdef __cplusplus
}
#endif

#endif
