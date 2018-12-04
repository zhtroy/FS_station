
#ifndef __SJA1000_H__
#define __SJA1000_H__

#ifdef __cplusplus
extern "C"{
#endif
#include "sja_common.h"
#include <xdc/runtime/Log.h>


/* sja1000 peli模式内部寄存器结构定义*/
typedef struct __sja1000_reg_peli
{
	volatile UINT8	mode_p;					/*00模式寄存器*/
	volatile UINT8  command_p;				/*01命令寄存器*/
	volatile UINT8  status_p; 				/*02状态寄存器*/
	volatile UINT8  interrupt_p;			/*03中断寄存器*/	
	volatile UINT8	intenable_p;			/*04中断使能寄存器*/
	volatile UINT8  resever0_p;				/*05保留*/
	volatile UINT8  btr0_p; 				/*06总线定时寄存器0 */
	volatile UINT8  btr1_p;					/*07总线定时寄存器1*/
	volatile UINT8  ocr_p;					/*08输出控制寄存器*/
	volatile UINT8	test_p;					/*09测试寄存器*/
	volatile UINT8  resever1_p;				/*0a保留*/
	volatile UINT8	alc_p;					/*0b仲裁丢失捕捉寄存器*/
	volatile UINT8	ecc_p;					/*0c错误代码捕捉寄存器*/
	volatile UINT8	emlr_p;					/*0d错误报警限制寄存器*/
	volatile UINT8	rxerr_p;				/*0eRX错误计数器寄存器*/
	volatile UINT8	txerr_p;				/*0fTX错误计数器寄存器*/
	volatile UINT8	tx0_rx0_acr0_p;		 	/*10发送缓冲器0/接收缓冲器0/验收代码寄存器0*/
	volatile UINT8	tx1_rx1_acr1_p;			/*11发送缓冲器1/接收缓冲器1/验收代码寄存器1*/
	volatile UINT8	tx2_rx2_acr2_p;		 	/*12发送缓冲器2/接收缓冲器2/验收代码寄存器2*/
	volatile UINT8	tx3_rx3_acr3_p;		 	/*13发送缓冲器3/接收缓冲器3/验收代码寄存器3*/
	volatile UINT8	tx4_rx4_amr0_p;			/*14发送缓冲器4/接收缓冲器4/验收屏蔽寄存器0*/
	volatile UINT8	tx5_rx5_amr1_p;			/*15发送缓冲器5/接收缓冲器5/验收屏蔽寄存器1*/
	volatile UINT8	tx6_rx6_amr2_p; 		/*16发送缓冲器6/接收缓冲器6/验收屏蔽寄存器2*/
	volatile UINT8	tx7_rx7_amr3_p; 		/*17发送缓冲器7/接收缓冲器7/验收屏蔽寄存器3*/
	volatile UINT8	tx8_rx8_p; 				/*18发送缓冲器8/接收缓冲器8*/
	volatile UINT8	tx9_rx9_p; 				/*19发送缓冲器9/接收缓冲器9*/
	volatile UINT8	tx10_rx10_p; 			/*1a发送缓冲器10/接收缓冲器10*/
	volatile UINT8	tx11_rx11_p; 			/*1b发送缓冲器11/接收缓冲器11*/
	volatile UINT8	tx12_rx12_p; 			/*1c发送缓冲器12/接收缓冲器12*/
	volatile UINT8	rxcounter_p;			/*1dRX信息计数器*/
	volatile UINT8	rxr_p;					/*1eRX缓冲器基地址*/
	volatile UINT8	cdr_p;					/*1f时钟分频寄存器*/
}SJA1000_REG_PELI;
/* sja1000 basic模式内部寄存器结构定义*/
typedef struct __sja1000_reg_basic
{
	volatile UINT8	control_b;				/*00模式寄存器*/
	volatile UINT8  command_b;				/*01命令寄存器*/
	volatile UINT8  status_b; 				/*02状态寄存器*/
	volatile UINT8  interrupt_b;			/*03中断寄存器*/	
	volatile UINT8	acr_b;					/*04中断使能寄存器*/
	volatile UINT8  amr_b;					/*05验收屏蔽寄存器*/
	volatile UINT8  btr0_b; 				/*06总线定时寄存器0 */
	volatile UINT8  btr1_b;					/*07总线定时寄存器1*/
	volatile UINT8  ocr_b;					/*08输出控制寄存器*/
	volatile UINT8	test_b;					/*09测试寄存器*/
	volatile UINT8  tx0_b;					/*0a发送缓冲区0*/
	volatile UINT8	tx1_b;					/*0b发送缓冲区1*/
	volatile UINT8	tx2_b;					/*0c发送缓冲区2*/
	volatile UINT8	tx3_b;					/*0d发送缓冲区3*/
	volatile UINT8	tx4_b;					/*0e发送缓冲区4*/
	volatile UINT8	tx5_b;					/*0f发送缓冲区5*/
	volatile UINT8	tx6_b;		 			/*10发送缓冲区6*/
	volatile UINT8	tx7_b;					/*11发送缓冲区7*/
	volatile UINT8	tx8_b;		 			/*12发送缓冲区8*/
	volatile UINT8	tx9_b;		 			/*13发送缓冲区9*/
	volatile UINT8	rx0_b;					/*14接收缓冲区0*/
	volatile UINT8	rx1_b; 					/*15接收缓冲区1*/
	volatile UINT8	rx2_b; 					/*16接收缓冲区2*/
	volatile UINT8	rx3_b; 					/*17接收缓冲区3*/
	volatile UINT8	rx4_b; 					/*18接收缓冲区4*/
	volatile UINT8	rx5_b; 					/*19接收缓冲区5*/
	volatile UINT8	rx6_b; 					/*1a接收缓冲区6*/
	volatile UINT8	rx7_b; 					/*1b接收缓冲区7*/
	volatile UINT8	rx8_b; 					/*1c接收缓冲区8*/
	volatile UINT8	rx9_b; 					/*1d接收缓冲区9*/
	volatile UINT8	resever_b; 				/*1e保留*/
	volatile UINT8	cdr_b;					/*1f时钟分频寄存器*/
}SJA1000_REG_BASIC;

/*SJA1000寄存器基地址*/
#define SJA1000_MODULE_BASE_ADDR		(0)/*逻辑模块偏移了0*/
/*目前支持的CAN波特率*/
#define	BAUDTATE_20K        (0)            /*0 ByteRate_20k*/           
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
#pragma pack(1)/*考虑字节对齐操作*/
/*sja1000内部寄存器结构定义*/
typedef union __sja1000_reg
{
	SJA1000_REG_BASIC regs_b;
	SJA1000_REG_PELI regs_p;
}SJA1000_REG;
/*sja1000数据结构定义*/
typedef struct _sja1000_data_obj
{
	UINT32  ID;/*报文ID*/
	UINT8 SendType;/*发送帧类型，=0时为正常发送，=1时为单次发送，=2时为自发自收，
	                        =3时为单次自发自收，只在此帧为发送帧时有意*/
	UINT8 RemoteFlag;/*是否是远程帧；1/远程帧 0/非远程帧*/
	UINT8 ExternFlag;/*是否是扩展帧,1/扩展帧 0/标准帧*/
	UINT8 DataLen;/*数据长度(<=8)，即Data 的长度*/
	INT8  Data[8];/*报文的数据；*/
}SJA1000_DATA_OBJ;
/*sja1000资源结构表*/
typedef struct _sja1000_PARAMS
{
	UINT32  sja1000Baseaddr;/*sja1000操作的基地址*/
	UINT32  sja1000BusWidth;/*sja1000总线宽度*/
	UINT32 	frameMode;/*CAN控制器的帧模式*/
	UINT32  acr;				/*验收屏蔽寄存器*/
	UINT32  amr;				/*验收掩码寄存器*/
	UINT32  bandRate;			/*波特率*/
}SJA1000_PARAMS;
/*
sja1000中断结构表
 */
 typedef struct _sja1000_int 
{
	UINT32 eventId;/*中断事件号*/
	UINT32 intTxRx;/*CAN接收发送中断使能*/
	ISRPFUNC irqCall;/*中断回调*/
	INT32  arg;/*中断回调函数参数*/
}SJA1000_INT;
/*sja1000诊断项定义*/
typedef struct _sja1000_diagnose
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
}SJA1000_DIAGNOSE;
/*sja1000设备维护的数据结构*/
typedef struct _sja1000_obj 
{
	SJA1000_PARAMS 	 sja1000_params_table;/*sja1000参数表*/
	SJA1000_INT		 sja1000_int_table;/*sja1000中断配置表*/
	SJA1000_DIAGNOSE sja1000_diagnose_table;/*sja1000诊断表*/
}SJA1000_OBJ;
#pragma pack()
/*sja1000 io控制宏*/
#define  CAN_BAUD_SET		(0)/*波特率设置*/
#define  CAN_BAUD_GET		(1)/*获取波特率*/
#define  CAN_ACCCODE		(2)/*滤波设置*/
#define  CAN_ACCMASK		(3)/*滤波掩码设置*/
#define  CAN_FILER			(4)/*滤波方式设置*/
#define	 CAN_MODE			(5)/*CAN模式选择(basicCAN peliCAN)*/
#define  CAN_FRAM_MODE		(6)/*CAN帧模式(标准帧 扩展帧)*/
#define  CAN_DEVINFO_GET	(7)/*can模块信息打印*/
#define  CAN_DEBUG			(8)/*can模块信息打印级别控制*/
#define  CAN_RESET			(9)/*CAN芯片硬件复位*/
#define  CAN_BUS_STATUS		(10)/*can总线状态*/
/*CAN滤波方式宏定义*/
#define FILTER_DUAL          (0<<16)        /*双滤波*/
#define FILTER_SINGLE        (1<<16)        /*单滤波*/
/*CAN工作方式宏定义。用于open函数中的flags参数配置*/
#define SJA100_OPERATION_NORMAL     0        /*正常模式*/
#define SJA100_OPERATION_RDONLY     1        /*只听模式*/
#define SJA100_OPERATION_LOOPBACK   2      /*自测模式*/
/*CAN帧格式定义*/
#define CAN_FRAME_TYPE_STD	 (0)	/*标准帧*/
#define CAN_FRAME_TYPE_EXT	 (1)	/*扩展帧*/
#define CAN_FRAME_TYPE_DATA	 (0)	/*数据帧*/
#define CAN_FRAME_TYPE_RTR	 (1)	/*远程帧*/
/*发送超时时间*/
#define SJA1000_WRITE_TIME_OUT    	(10000UL)/*超时时间*/

extern INT32 sja1000_dbg_level;
/*调试信息控制*/
#define SJA1000_DEBUG_LOG(level,fmt,arg0,arg1,arg2,arg3,arg4,arg5) do{\
		if (level <= sja1000_dbg_level) { \
			logMsg(0,"\rcan:%d\n",arg0); \
			logMsg(1,"\r%s(%d):"fmt"\n",__FUNCTION__,__LINE__,arg1,arg2,arg3,arg4,arg5);  \
		}\
	}while(0)
/*sja1000底层驱动函数声明*/
extern INT32 sja1000Open(void * sja1000DrvObj);
extern INT32 sja1000Close(void * sja1000DrvObj);
extern INT32 sja1000Write(void * sja1000DrvObj,void * data);
extern INT32 sja1000Read(void * sja1000DrvObj,void * data);
extern INT32 sja1000IoCtl(void * sja1000DrvObj,UINT8 funcNo,UINT32 *arg);
extern void sja1000Isr(INT32 arg);

#ifdef __cplusplus
}
#endif

#endif

