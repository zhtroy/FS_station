#ifndef __SJA_COMMON__
#define __SJA_COMMON__

#ifndef TRUE
#  define TRUE		1
#endif

#ifndef FALSE
#  define FALSE		0
#endif

#ifndef NULL
#define NULL		0
#endif

#ifndef null
#define null		0
#endif

#define DBG_LEVEL   3
#define DBG_OFF     0
#define DBG_ERROR   1
#define DBG_INVALID 2
#define DBG_STATUS  3


#define DEV_OK                      0
#define DEV_ON_INSTANCE             (-1)
#define DEV_CLOSED                  (-2)
#define DEV_ON_CONNECT              (-3)
#define DEV_OPENED                  (-4)
#define DEV_HAND_NULL               (-5)
#define DEV_NO_EXIST                (-6)
#define DEV_ENTER_RESET_FAIL        (-7)
#define DEV_REC_FIFO_EMPTY          (-8)
#define DEV_SEBD_TYPE_ERROR         (-9)
#define DEV_SEND_BUSY               (-10)
#define DEV_SEND_TIME_OUT           (-11)
#define DEV_WORK_MODE_NONSUPPORT    (-12)
#define DEV_BAND_RATE_NONSUPPORT    (-13)
#define DEV_EXIT_RESET_FAIL         (-14)
#define DEV_FILTER_PARAM_ERROR      (-15)


typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long UINT32;
typedef unsigned long long UINT64;

typedef char INT8;
typedef short INT16;
typedef long INT32;
typedef long long INT64;

typedef void (*ISRPFUNC)(INT32 arg,INT32 event);

#define xIoRead(baseAddr,regAddr,data) \
	(*data = *((volatile INT16 *)baseAddr + regAddr))

#define xIoWrite(baseAddr,regAddr,data) \
	(*((volatile INT16 *)baseAddr + regAddr) = data)

extern void logMsg(INT32 index,INT8 *fmt, ...);

 /*sja1000底层驱动函数声明*/
extern INT32 sja1000Open(void * sja1000DrvObj);
extern INT32 sja1000Close(void * sja1000DrvObj);
extern INT32 sja1000Write(void * sja1000DrvObj,void * data);
extern INT32 sja1000Read(void * sja1000DrvObj,void * data);
extern INT32 sja1000IoCtl(void * sja1000DrvObj,UINT8 funcNo,UINT32 *arg);
extern void sja1000Isr(INT32 arg);

#endif
