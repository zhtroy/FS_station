#include "canModule.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <xdc/runtime/System.h>
#include "uartStdio.h"


/*
@brief:can底层接口注册  片级需要实现以下接口
*/
CAN_FUNC_TABLE can_func_table[]=
{
	/*sja1000设备*/
	#ifdef ZMD_SJA1000_MODULE
	/*设备名 操作设备的相关接口*/
	{"/sja1000/",sja1000Open,sja1000Write,sja1000Read,sja1000Close,sja1000IoCtl,sja1000Isr},
	#endif
	/*ecan设备*/
	#ifdef ECAN_MODULE
	/*设备名 操作设备的相关接口*/
	{"/ecan/",ecanOpen,ecanWrite,ecanRead,ecanClose,ecanIoCtl,ecanIsr},
	#endif
	/*空设备*/
	{"null",null,null,null,null,null,null}
};
/*实际有效设备个数*/
#define CAN_DEV_NUM (NELEMENTS(can_func_table)-1)
/*
@brief:CAN底层接口参数信息表 唯一需要修改的表
sja1000:
总线基地址:与逻辑通信的片选地址+模块地址
总线位宽:与逻辑相接的片选位宽
CAN帧模式:标准帧(0)扩展帧(1)
滤波屏蔽码:
	单滤波:标准帧(0-11),扩展帧(0-29)
	双滤波:标准帧(滤波器1:0-11滤波器2:16-27)扩展帧:(滤波器1:(0-15)滤波器2:(16-31))对应ID的高16位
滤波掩码:相应位为0表示接受滤波，相应位为1表示不做滤波
	单滤波:标准帧(0-11),扩展帧(0-29)
	双滤波:标准帧(滤波器1:0-11滤波器2:16-27)扩展帧:(滤波器1:(0-15)滤波器2:(16-31))对应ID的高16位
波特率:0-12
中断事件号:内部事件:0-128 外部事件:200-296
中断号:4-15
中断类型:上升沿、下降沿
BANK ID:6713与6748含有
外部中断控制器相关参数需要填写以下参数:
中断源捕获模式:1(下降沿)2(上升沿)
中断输出模式:1(下降沿)2(上升沿)
中断路由规则:(0-7)表示从外部中断模块那个管脚输出
CAN设备相关中断参数:
CAN中断类型:bit0接收中断，bit1发送中断
中断回调函数、中断回调参数:模块底层维护
ecan:
ecan片内通道:表示片内第几路设备
GPIO复用管脚:于硬件相关的管脚定义
CAN帧模式:标准帧(0)扩展帧(1)
滤波屏蔽码:标准帧(0-11),扩展帧(0-29)
滤波掩码:相应位为0表示接受滤波，相应位为1表示不做滤波 标准帧(0-11),扩展帧(0-29)
*/


CAN_CFG_TABLE can_cfg_table[]=
{
	#if defined ZMD_SJA1000_MODULE
	/*设备1资源定义*/
	{	
		/*设备名*/
		"/sja1000/",
		/*设备序列号*/
		0,
		{
		/*总线基地址 总线位宽(8/16/32/64) CAN帧模式 滤波屏蔽码 滤波掩码 波特率设置*/
		{0x60001000,16,CAN_FRAME_TYPE_EXT,0xffffffff,0xffffffff,BAUDTATE_500K},
		/*中断事件号 路由规则    CAN中断类型 中断回调函数(模块维护) 回调参数*/
		{53,RIE|TIE,NULL,null}
		}
	},
	/*设备2资源定义*/
	{	
		/*设备名*/
		"/sja1000/",
		/*设备序列号*/
		1,
		{
		/*总线基地址 总线位宽(8/16/32/64) CAN帧模式 滤波屏蔽码 滤波掩码 波特率设置*/
		{0x60001200,16,CAN_FRAME_TYPE_EXT,0xffffffff,0xffffffff,BAUDTATE_500K},
		/*中断事件号 路由规则    CAN中断类型 中断回调函数(模块维护) 回调参数*/
		{54,RIE|TIE,NULL,null}
		}
	},
	#endif
	#ifdef ZMD_ECAN_MODULE
	/*设备1资源定义*/
	{	
		/*设备名*/
		"/ecan/",
		/*设备序列号*/
		0,
		{
		/*ecan片内通道 GPIO复用管脚 CAN帧模式 滤波屏蔽码 滤波掩码 波特率设置*/
		{0,0,CAN_FRAME_TYPE_STD,0x40,0xffffffff,BAUDTATE_1000K},
		/*ecan相关中断参数直接给零 CAN中断类型 中断回调函数(模块维护) 回调参数*/
		{null,RIE|TIE,NULL,null}
		}
	},
	/*设备2资源定义*/
	{	
		/*设备名*/
		"/ecan/",
		/*设备序列号*/
		1,
		{
		/*ecan片内通道 GPIO复用管脚 CAN帧模式 滤波屏蔽码 滤波掩码 波特率设置*/
		{1,3,CAN_FRAME_TYPE_STD,0x40,0xffffffff,BAUDTATE_1000K},
		/*ecan相关中断参数直接给零 CAN中断类型 中断回调函数(模块维护) 回调参数*/
		{null,RIE|TIE,NULL,null}
		}
	},
	#endif
	/*空设备*/
	{	
		"null",
		0,
		{
		{null,null,null,null,null},
		{null,null,NULL,null}
		}
	}
};
/*实际有效设备资源表*/
#define CAN_CFG_NUM (NELEMENTS(can_cfg_table)-1)
/*设备驱动资源表*/
CAN_DEV_TABLE can_dev_table[CAN_CFG_NUM]={0};
/*****************************************************************************
 * 函数名称: INT32 canTableInit(void)
 * 函数说明: 初始化can设备维护表 该表由项目开发人员必须调用 否则资源不能清空
 * 输入参数: void
 * 输出参数: 无
 * 返 回 值: 0(成功)/负数(失败)
 * 备注: 
*****************************************************************************/
INT32 canTableInit(void)
{
	UINT32 i;
	for(i=0;i<CAN_CFG_NUM;i++)
	{
		memset((INT8 *)&can_dev_table[i],0,sizeof(CAN_DEV_TABLE));
		memset((INT8 *)&can_cfg_table[i].hwCfg.can_diagnose_table,0,sizeof(CAN_DIAGNOSE));
	}
	return 0;
}
/*****************************************************************************
 * 函数名称: INT32 canOpen(UINT8 devsNum,void * canIsrCall,INT32 arg)
 * 函数说明: 打开并初始化模块化中的设备 该接口会对设备资源表中定义的模块进行初始化操作
 * 输入参数: 
 *			devsNum:需要打开的设备设备号
 *			canIsrCall:中断回调函数
 *			arg:中断回调函数参数
 * 输出参数: 无
 * 返 回 值: 0(成功)/负数(失败)
 * 备注: 
*****************************************************************************/
INT32 canOpen(UINT8 devsNum,ISRPFUNC canIsrCall,INT32 arg)
{
	UINT32 i;
	INT32 status=0;
	UINT32 drvIndex=0,paramIndex=0;
	if(devsNum>=CAN_CFG_NUM)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d实例未找到",DEV_ON_INSTANCE,devsNum,0,0,0,0);
		return DEV_ON_INSTANCE;
	}
	/*实例与驱动匹配(同一个驱动可以匹配多个实例)*/
	for(i=0;i<CAN_DEV_NUM;i++)
	{
		if(strcmp(can_cfg_table[devsNum].hwName,can_func_table[i].hwName)==0)
		{
			drvIndex=i;
			paramIndex=devsNum;
			break;
		}
	}
	if(drvIndex>CAN_DEV_NUM)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d驱动未注册",DEV_ON_CONNECT,devsNum,0,0,0,0);
		return DEV_ON_CONNECT;
	}
	if(can_dev_table[devsNum].devStatus==CAN_DEV_OPENED)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d已经打开",DEV_OPENED,devsNum,0,0,0,0);
		return DEV_OPENED;
	}
	if(canIsrCall!=NULL)
	{
		/*注册中断回调函数*/
		can_cfg_table[devsNum].hwCfg.can_int_cfg_table.irqCall=canIsrCall;
		can_cfg_table[devsNum].hwCfg.can_int_cfg_table.arg=arg;
	}
	can_dev_table[devsNum].devsNum=devsNum;
	can_dev_table[devsNum].drvIndex=drvIndex;
	can_dev_table[devsNum].paramIndex=paramIndex;
	/*获取设备名 格式:/设备名/设备单元*/
	sprintf(can_dev_table[devsNum].hwName,"%s%d",can_cfg_table[devsNum].hwName,can_cfg_table[devsNum].devsNum);

    /*硬件复位CAN控制器*/
    canHardWareReset(devsNum);
    
	status=can_func_table[drvIndex].hwCanOpen((CAN_PARAMS_INT_TABLE *)&can_cfg_table[paramIndex].hwCfg);
	if(status==0)
	{
		can_dev_table[devsNum].devStatus=CAN_DEV_OPENED;
		return DEV_OK;
	}
	return status;
}
/*****************************************************************************
 * 函数名称: INT32 canClose(UINT8 devsNum)
 * 函数说明: 关闭并释放模块的资源 该接口会对设备资源表中定义的模块进行初始化操作
 * 输入参数: 
 *			devsNum:需要打开的设备设备号
 * 输出参数: 无
 * 返 回 值: 0(成功)/负数(失败)
 * 备注: 
*****************************************************************************/
INT32 canClose(UINT8 devsNum)
{
	INT32 status=0;
	UINT32 drvIndex=0,paramIndex=0;
	if(devsNum>=CAN_CFG_NUM)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d实例未找到",DEV_ON_INSTANCE,devsNum,0,0,0,0);
		return DEV_ON_INSTANCE;
	}
	if(can_dev_table[devsNum].devStatus==CAN_DEV_CLOSED)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d未打开",DEV_CLOSED,devsNum,0,0,0,0);
		return DEV_CLOSED;
	}
	drvIndex=can_dev_table[devsNum].drvIndex;
	paramIndex=can_dev_table[devsNum].paramIndex;
	status=can_func_table[drvIndex].hwCanClose((CAN_PARAMS_INT_TABLE *)&can_cfg_table[paramIndex].hwCfg);
	/*清空该设备的设备资源表中的所有配置信息*/
	memset((INT8 *)&can_dev_table[devsNum],0,sizeof(CAN_DEV_TABLE));
	return status;
}
/*****************************************************************************
 * 函数名称: INT32 canWrite(UINT8 devsNum,CAN_DATA_OBJ * canData)
 * 函数说明: 设备发送数据接口
 * 输入参数: 
 *			devsNum:需要打开的设备设备号
 *			canData:数据
 * 输出参数: 无
 * 返 回 值: 正数(实际发送的长度)/负数(失败)
 * 备注: 
*****************************************************************************/
INT32 canWrite(UINT8 devsNum,CAN_DATA_OBJ * canData)
{
	INT32 status=0;
	UINT32 drvIndex=0,paramIndex=0;
	if(devsNum>=CAN_CFG_NUM)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d实例未找到",DEV_ON_INSTANCE,devsNum,0,0,0,0);
		return DEV_ON_INSTANCE;
	}
	if(can_dev_table[devsNum].devStatus==CAN_DEV_CLOSED)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d未打开",DEV_CLOSED,devsNum,0,0,0,0);
		return DEV_CLOSED;
	}
	drvIndex=can_dev_table[devsNum].drvIndex;
	paramIndex=can_dev_table[devsNum].paramIndex;
	status=can_func_table[drvIndex].hwCanWrite((CAN_PARAMS_INT_TABLE *)&can_cfg_table[paramIndex].hwCfg,canData);
	return status;
}
/*****************************************************************************
 * 函数名称: INT32 canRead(UINT8 devsNum,CAN_DATA_OBJ * canData)
 * 函数说明: 设备接收数据接口
 * 输入参数: 
 *			devsNum:需要打开的设备设备号
 *			canData:数据
 * 输出参数: 无
 * 返 回 值: 正数(实际接收的长度)/负数(失败)
 * 备注: 
*****************************************************************************/
INT32 canRead(UINT8 devsNum,CAN_DATA_OBJ * canData)
{
	INT32 status=0;
	UINT32 drvIndex=0,paramIndex=0;
	if(devsNum>=CAN_CFG_NUM)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d实例未找到",DEV_ON_INSTANCE,devsNum,0,0,0,0);
		return DEV_ON_INSTANCE;
	}
	if(can_dev_table[devsNum].devStatus==CAN_DEV_CLOSED)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d未打开",DEV_CLOSED,devsNum,0,0,0,0);
		return DEV_CLOSED;
	}
	drvIndex=can_dev_table[devsNum].drvIndex;
	paramIndex=can_dev_table[devsNum].paramIndex;
	status=can_func_table[drvIndex].hwCanRead((CAN_PARAMS_INT_TABLE *)&can_cfg_table[paramIndex].hwCfg,canData);
	return status;
}
/*****************************************************************************
 * 函数名称: INT32 canIoCtl(UINT8 devsNum,UINT32 funcNo,UINT32 *arg)
 * 函数说明: 设备发送数据接口
 * 输入参数: 
 *			devsNum:设备号
 *			funcNo:控制选项
 *			arg:控制参数
 * 输出参数: 无
 * 返 回 值: 0(成功)/负数(失败)
 * 备注: 
*****************************************************************************/
INT32 canIoCtl(UINT8 devsNum,UINT8 funcNo,UINT32 *arg)
{
	INT32 status=0;
	UINT32 drvIndex=0,paramIndex=0;
	if(devsNum>=CAN_CFG_NUM)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d实例未找到",DEV_ON_INSTANCE,devsNum,0,0,0,0);
		return DEV_ON_INSTANCE;
	}
	if(can_dev_table[devsNum].devStatus==CAN_DEV_CLOSED)
	{
		CAN_DEBUG_LOG(DBG_INVALID,"设备%d未打开",DEV_CLOSED,devsNum,0,0,0,0);
		return DEV_CLOSED;
	}
	drvIndex=can_dev_table[devsNum].drvIndex;
	paramIndex=can_dev_table[devsNum].paramIndex;
	status=can_func_table[drvIndex].hwCanCtl((CAN_PARAMS_INT_TABLE *)&can_cfg_table[paramIndex].hwCfg,funcNo,arg);
	return status;
}


void canIsr(INT32 arg)
{
    INT32 devsNum = arg;
    UINT32 paramIndex=0;
	paramIndex=can_dev_table[devsNum].paramIndex;
    sja1000Isr((INT32)&can_cfg_table[paramIndex].hwCfg);
}

/*****************************************************************************
 * 函数名称: INT32 canHardWareReset(UINT8 devsNum)
 * 函数说明: CAN控制器硬件复位
 * 输入参数:
 *          devsNum:设备号
 * 输出参数: 无
 * 返 回 值: 0(成功)/-1(失败)
 * 备注:
*****************************************************************************/
static canHardWareReset(UINT8 devsNum)
{
    UINT8 Reg;
    xIoRead(CAN_RESET_ADDR,0, &Reg);
    xIoWrite(CAN_RESET_ADDR,0, Reg | (1 << devsNum));
    xIoWrite(CAN_RESET_ADDR,0, Reg & (~(1 << devsNum)));
}

void canInitBuffer(UINT8 devsNum)
{
    CAN_BUFFER_OBJ *LoopBuf = &(can_cfg_table[devsNum].lpBuf);
    
    LoopBuf->ReadPoint = 0;
    
    LoopBuf->WritePoint = 0;
}
UINT8 canBufferIsEmpty(UINT8 devsNum)
{
    CAN_BUFFER_OBJ *LoopBuf = &(can_cfg_table[devsNum].lpBuf);
    
    return (LoopBuf->ReadPoint == LoopBuf->WritePoint) ? 1 :0; 
}

CAN_DATA_OBJ* canPopBuffer(UINT8 devsNum)
{
    CAN_BUFFER_OBJ *LoopBuf = &(can_cfg_table[devsNum].lpBuf);
    
    CAN_DATA_OBJ * canData = &(LoopBuf->LoopBuffer[LoopBuf->ReadPoint]);
    
    LoopBuf->ReadPoint = (LoopBuf->ReadPoint+1)%CAN_BUFFER_MAX_DEPTH;
    
    /*返回数据Data地址*/
    return canData;
}

CAN_DATA_OBJ * canPushBuffer(UINT8 devsNum)
{
    CAN_BUFFER_OBJ *LoopBuf = &(can_cfg_table[devsNum].lpBuf);
    CAN_DATA_OBJ * canData = &(LoopBuf->LoopBuffer[LoopBuf->WritePoint]);
    
    LoopBuf->WritePoint = (LoopBuf->WritePoint+1)%CAN_BUFFER_MAX_DEPTH;
    
    /*返回数据Data地址*/
    return canData;
}


void canHardIntMask(UINT8 devsNum)
{
    UINT8 Reg;
    Reg = *(volatile UINT16 *) (CAN_INT_MASK_ADDR);
    *(volatile UINT16 *) (CAN_INT_MASK_ADDR) =  Reg | (1 << devsNum);
}
    
void canHardIntUnmask(UINT8 devsNum)
{
    UINT8 Reg;
    Reg = *(volatile UINT16 *) (CAN_INT_MASK_ADDR);
    *(volatile UINT16 *) (CAN_INT_MASK_ADDR) =  Reg & (~(1 << devsNum));
}

void logMsg(INT32 index,INT8 *fmt, ...)
{
	va_list vp;
    INT8 func[128];
    INT8 * str = func;
    va_start(vp,fmt);
    vsprintf(str,fmt,vp);
    UARTprintf("%d:%s",index,str);
}

