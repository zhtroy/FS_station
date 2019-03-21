#ifndef __SJA1000_H__
#define __SJA1000_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "common.h"

/* 类型定义 */
/* 中断函数指针 */
typedef void (*isrFuncPtr_t)(int32_t arg,int32_t event);

/* sja1000 peli模式内部寄存器结构定义*/
typedef struct __sja1000_reg_peli
{
	volatile uint8_t    mode_p;					/*00模式寄存器*/
	volatile uint8_t    command_p;				/*01命令寄存器*/
	volatile uint8_t    status_p; 				/*02状态寄存器*/
	volatile uint8_t    interrupt_p;			/*03中断寄存器*/	
	volatile uint8_t	intenable_p;			/*04中断使能寄存器*/
	volatile uint8_t    resever0_p;				/*05保留*/
	volatile uint8_t    btr0_p; 				/*06总线定时寄存器0 */
	volatile uint8_t    btr1_p;					/*07总线定时寄存器1*/
	volatile uint8_t    ocr_p;					/*08输出控制寄存器*/
	volatile uint8_t	test_p;					/*09测试寄存器*/
	volatile uint8_t    resever1_p;				/*0a保留*/
	volatile uint8_t	alc_p;					/*0b仲裁丢失捕捉寄存器*/
	volatile uint8_t	ecc_p;					/*0c错误代码捕捉寄存器*/
	volatile uint8_t	emlr_p;					/*0d错误报警限制寄存器*/
	volatile uint8_t	rxerr_p;				/*0eRX错误计数器寄存器*/
	volatile uint8_t	txerr_p;				/*0fTX错误计数器寄存器*/
	volatile uint8_t	tx0_rx0_acr0_p;		 	/*10发送缓冲器0/接收缓冲器0/验收代码寄存器0*/
	volatile uint8_t	tx1_rx1_acr1_p;			/*11发送缓冲器1/接收缓冲器1/验收代码寄存器1*/
	volatile uint8_t	tx2_rx2_acr2_p;		 	/*12发送缓冲器2/接收缓冲器2/验收代码寄存器2*/
	volatile uint8_t	tx3_rx3_acr3_p;		 	/*13发送缓冲器3/接收缓冲器3/验收代码寄存器3*/
	volatile uint8_t	tx4_rx4_amr0_p;			/*14发送缓冲器4/接收缓冲器4/验收屏蔽寄存器0*/
	volatile uint8_t	tx5_rx5_amr1_p;			/*15发送缓冲器5/接收缓冲器5/验收屏蔽寄存器1*/
	volatile uint8_t	tx6_rx6_amr2_p; 		/*16发送缓冲器6/接收缓冲器6/验收屏蔽寄存器2*/
	volatile uint8_t	tx7_rx7_amr3_p; 		/*17发送缓冲器7/接收缓冲器7/验收屏蔽寄存器3*/
	volatile uint8_t	tx8_rx8_p; 				/*18发送缓冲器8/接收缓冲器8*/
	volatile uint8_t	tx9_rx9_p; 				/*19发送缓冲器9/接收缓冲器9*/
	volatile uint8_t	tx10_rx10_p; 			/*1a发送缓冲器10/接收缓冲器10*/
	volatile uint8_t	tx11_rx11_p; 			/*1b发送缓冲器11/接收缓冲器11*/
	volatile uint8_t	tx12_rx12_p; 			/*1c发送缓冲器12/接收缓冲器12*/
	volatile uint8_t	rxcounter_p;			/*1dRX信息计数器*/
	volatile uint8_t	rxr_p;					/*1eRX缓冲器基地址*/
	volatile uint8_t	cdr_p;					/*1f时钟分频寄存器*/
}sja1000RegPeil_t;

/* sja1000 basic模式内部寄存器结构定义*/
typedef struct __sja1000_reg_basic
{
	volatile uint8_t	control_b;				/*00模式寄存器*/
	volatile uint8_t  command_b;				/*01命令寄存器*/
	volatile uint8_t  status_b; 				/*02状态寄存器*/
	volatile uint8_t  interrupt_b;			/*03中断寄存器*/	
	volatile uint8_t	acr_b;					/*04中断使能寄存器*/
	volatile uint8_t  amr_b;					/*05验收屏蔽寄存器*/
	volatile uint8_t  btr0_b; 				/*06总线定时寄存器0 */
	volatile uint8_t  btr1_b;					/*07总线定时寄存器1*/
	volatile uint8_t  ocr_b;					/*08输出控制寄存器*/
	volatile uint8_t	test_b;					/*09测试寄存器*/
	volatile uint8_t  tx0_b;					/*0a发送缓冲区0*/
	volatile uint8_t	tx1_b;					/*0b发送缓冲区1*/
	volatile uint8_t	tx2_b;					/*0c发送缓冲区2*/
	volatile uint8_t	tx3_b;					/*0d发送缓冲区3*/
	volatile uint8_t	tx4_b;					/*0e发送缓冲区4*/
	volatile uint8_t	tx5_b;					/*0f发送缓冲区5*/
	volatile uint8_t	tx6_b;		 			/*10发送缓冲区6*/
	volatile uint8_t	tx7_b;					/*11发送缓冲区7*/
	volatile uint8_t	tx8_b;		 			/*12发送缓冲区8*/
	volatile uint8_t	tx9_b;		 			/*13发送缓冲区9*/
	volatile uint8_t	rx0_b;					/*14接收缓冲区0*/
	volatile uint8_t	rx1_b; 					/*15接收缓冲区1*/
	volatile uint8_t	rx2_b; 					/*16接收缓冲区2*/
	volatile uint8_t	rx3_b; 					/*17接收缓冲区3*/
	volatile uint8_t	rx4_b; 					/*18接收缓冲区4*/
	volatile uint8_t	rx5_b; 					/*19接收缓冲区5*/
	volatile uint8_t	rx6_b; 					/*1a接收缓冲区6*/
	volatile uint8_t	rx7_b; 					/*1b接收缓冲区7*/
	volatile uint8_t	rx8_b; 					/*1c接收缓冲区8*/
	volatile uint8_t	rx9_b; 					/*1d接收缓冲区9*/
	volatile uint8_t	resever_b; 				/*1e保留*/
	volatile uint8_t	cdr_b;					/*1f时钟分频寄存器*/
}sja1000RegBasic_t;


#pragma pack(1)/*考虑字节对齐操作*/

/*sja1000内部寄存器结构定义*/
typedef union __sja1000_reg
{
	sja1000RegBasic_t regs_b;
	sja1000RegPeil_t regs_p;
}sja1000Reg_t;

/*sja1000数据结构定义*/
typedef struct _sja1000_data_obj
{
	uint32_t  ID;/*报文ID*/
	uint8_t SendType;/*发送帧类型，=0时为正常发送，=1时为单次发送，=2时为自发自收，
	                        =3时为单次自发自收，只在此帧为发送帧时有意*/
	uint8_t RemoteFlag;/*是否是远程帧；1/远程帧 0/非远程帧*/
	uint8_t ExternFlag;/*是否是扩展帧,1/扩展帧 0/标准帧*/
	uint8_t DataLen;/*数据长度(<=8)，即Data 的长度*/
	int8_t  Data[8];/*报文的数据；*/
}sja1000DataObj_t;

/*sja1000资源结构表*/
typedef struct _sja1000_PARAMS
{
	uint32_t  sja1000Baseaddr;/*sja1000操作的基地址*/
	uint32_t  sja1000BusWidth;/*sja1000总线宽度*/
	uint32_t 	frameMode;/*CAN控制器的帧模式*/
	uint32_t  acr;				/*验收屏蔽寄存器*/
	uint32_t  amr;				/*验收掩码寄存器*/
	uint32_t  bandRate;			/*波特率*/
}sja1000Params_t;

/*sja1000中断结构表*/
 typedef struct _sja1000_int 
{
	uint32_t eventId;/*中断事件号*/
	uint32_t intTxRx;/*CAN接收发送中断使能*/
	isrFuncPtr_t irqCall;/*中断回调*/
	int32_t  arg;/*中断回调函数参数*/
}sja1000Int_t;

/*sja1000诊断项定义*/
typedef struct _sja1000_diagnose
{
	uint32_t   nIntCount;	/*产生的中断次数*/
	uint32_t   nIntRecvCount;	/*产生的接收中断次数*/
	uint32_t   nIntSendCount;	/*产生的发送中断次数*/
	uint32_t   nWriteCount;	/*应用调用写的次数*/
	uint32_t   nReadCount;   /*应用调用读的次数*/
	uint32_t   nBEICount; /*总线错误中断次数*/
	uint32_t   nEICount;	/*错误报警中断次数*/
	uint32_t   nDOICount;	/*数据溢出中断次数*/
	uint32_t   nEPICount;	/*错误消极中断次数*/
	uint32_t   nALIICount;	/*仲裁丢失中断次数*/
	uint8_t    nALErrValue;  /*最后一次仲裁丢失的错误代码值*/
	uint8_t    nErrValue;  /*最后一次的错误代码值*/	
	uint8_t    nTxErrValue;  /*发送错误计数*/
	uint8_t    nRxErrValue;  /*接收错误计数*/		
}sja1000Diagnose_t;

/*sja1000设备维护的数据结构*/
typedef struct _sja1000_obj 
{
	sja1000Params_t 	 sja1000_params_table;/*sja1000参数表*/
	sja1000Int_t		 sja1000_int_table;/*sja1000中断配置表*/
	sja1000Diagnose_t sja1000_diagnose_table;/*sja1000诊断表*/
}sja1000Obj_t;
#pragma pack()



/* 宏定义 */
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

/*调试信息控制*/
#define SJA1000_DEBUG_LOG(type,fmt,arg1,arg2,arg3,arg4,arg5) do{\
			FSZX_DEBUG_LOG(type,"\r0:can:%d\n",arg1, 0, 0, 0, 0, 0); \
			FSZX_DEBUG_LOG(type,"\r1:%s(%d):"fmt"\n",__FUNCTION__,__LINE__,arg2,arg3,arg4,arg5);  \
	}while(0)


/*sja1000底层驱动函数声明*/
int32_t sja1000Open(void * sja1000DrvObj);
int32_t sja1000Close(void * sja1000DrvObj);
int32_t sja1000Write(void * sja1000DrvObj,void * data);
int32_t sja1000Read(void * sja1000DrvObj,void * data);
int32_t sja1000IoCtl(void * sja1000DrvObj,uint8_t funcNo,uint32_t *arg);
void sja1000Isr(int32_t arg);

#ifdef __cplusplus
}
#endif

#endif

