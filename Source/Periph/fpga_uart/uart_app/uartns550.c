/***************************** Include Files ********************************/
#include "uartns550.h"

/************************** Constant Definitions ****************************/

uartCfgTable_t uartCfgTable[] = {

     /*设备0资源配置*/
    {
         /*设备号,设备基地址ַ,设备驱动时钟<Hz>*/
         0,(SOC_EMIFA_CS2_ADDR+0x200),80000000,
         /*波特率,数据长度,校验位,ֹͣ停止位*/
         {115200,XUN_FORMAT_8_BITS,XUN_FORMAT_NO_PARITY,XUN_FORMAT_1_STOP_BIT},
         /*设备OPTIONS*/
         XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE,
         /*FIFO触发深度*/
         XUN_FIFO_TRIGGER_08
    },
     
     /*设备1资源配置*/
    {
         /*设备号,设备基地址ַ,设备驱动时钟<Hz>*/
         1,(SOC_EMIFA_CS2_ADDR+0x400),80000000,
         /*波特率,数据长度,校验位,ֹͣ停止位*/
         {115200,XUN_FORMAT_8_BITS,XUN_FORMAT_NO_PARITY,XUN_FORMAT_1_STOP_BIT},
         /*设备OPTIONS*/
         XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE,
         /*FIFO触发深度*/
         XUN_FIFO_TRIGGER_08
    },
     
     /*设备2资源配置*/
    {
         /*设备号,设备基地址ַ,设备驱动时钟<Hz>*/
         2,(SOC_EMIFA_CS2_ADDR+0x600),80000000,
         /*波特率,数据长度,校验位,ֹͣ停止位*/
         {115200,XUN_FORMAT_8_BITS,XUN_FORMAT_ODD_PARITY,XUN_FORMAT_1_STOP_BIT},
         /*设备OPTIONS*/
         XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE,
         /*FIFO触发深度*/
         XUN_FIFO_TRIGGER_08
    },
    
     /*设备3资源配置*/
    {
         /*设备号,设备基地址ַ,设备驱动时钟<Hz>*/
         3,(SOC_EMIFA_CS2_ADDR+0x800),80000000,
         /*波特率,数据长度,校验位,ֹͣ停止位*/
         {115200,XUN_FORMAT_8_BITS,XUN_FORMAT_NO_PARITY,XUN_FORMAT_1_STOP_BIT},
         /*设备OPTIONS*/
         XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE,
         /*FIFO触发深度*/
         XUN_FIFO_TRIGGER_08
    },
    
    /*设备4资源配置*/
    {
         /*设备号,设备基地址ַ,设备驱动时钟<Hz>*/
         4,(SOC_EMIFA_CS2_ADDR+0xA00),80000000,
         /*波特率,数据长度,校验位,ֹͣ停止位*/
         {115200,XUN_FORMAT_8_BITS,XUN_FORMAT_NO_PARITY,XUN_FORMAT_1_STOP_BIT},
         /*设备OPTIONS*/
         XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE,
         /*FIFO触发深度*/
         XUN_FIFO_TRIGGER_08
    },
    
     /*设备5资源配置*/
    {
         /*设备号,设备基地址ַ,设备驱动时钟<Hz>*/
         5,(SOC_EMIFA_CS2_ADDR+0xC00),80000000,
         /*波特率,数据长度,校验位,ֹͣ停止位*/
         {115200,XUN_FORMAT_8_BITS,XUN_FORMAT_NO_PARITY,XUN_FORMAT_1_STOP_BIT},
         /*设备OPTIONS*/
         XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE,
         /*FIFO触发深度*/
         XUN_FIFO_TRIGGER_08
    }
};

/*获取设备数量*/
#define UART_CFG_NUM (NELEMENTS(uartCfgTable)-1)

/*****************************************************************************
 * 函数名称: int32_t UartNs550Init(uint16_t deviceNum,XUartNs550_Handler funcPtr)
 * 函数说明: 初始化UART设备维护表 该表由项目开发人员必须调用 否则资源不能清空
 * 输入参数:
 *        deviceNum：设备号
 *        funcPtr：中断回调函数
 * 输出参数: 无
 * 返 回 值: 0(成功)/负数(失败)
 * 备注:
*****************************************************************************/
int32_t UartNs550Init(uint16_t deviceNum,XUartNs550_Handler funcPtr)
{
    int32_t status;
    XUartNs550 * instancePtr;
    
    if(deviceNum > UART_CFG_NUM)
    {
        return XST_FAILURE;
    }
    
    instancePtr = &(uartCfgTable[deviceNum].instance);
    
    /* 设备硬件复位 */
    UartNs550HardReset(deviceNum);
    
    /* 根据配置表初始化成员变量 */
	instancePtr->BaseAddress = uartCfgTable[deviceNum].baseAddress;
	instancePtr->InputClockHz = uartCfgTable[deviceNum].inputClockHz;

	/*
	 * Initialize the instance data to some default values and setup
	 * a handler and CallBackRef.
 	 */
 	
	instancePtr->Handler = funcPtr;
    //instancePtr->CallBackRef = instancePtr;
    instancePtr->CallBackRef = &(uartCfgTable[deviceNum].deviceNum);
    
	instancePtr->SendBuffer.NextBytePtr = NULL;
	instancePtr->SendBuffer.RemainingBytes = 0;
	instancePtr->SendBuffer.RequestedBytes = 0;
	instancePtr->ReceiveBuffer.NextBytePtr = NULL;
	instancePtr->ReceiveBuffer.RemainingBytes = 0;
	instancePtr->ReceiveBuffer.RequestedBytes = 0;

    /*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	instancePtr->IsReady = XIL_COMPONENT_IS_READY;

    status = XUartNs550_SetDataFormat(instancePtr, 
        &(uartCfgTable[deviceNum].dataFormat));
    
	if (status != XST_SUCCESS) {
		instancePtr->IsReady = 0;
		return status;
	}

    status = XUartNs550_SetOptions(instancePtr, 
        uartCfgTable[deviceNum].options);
    
	if (status != XST_SUCCESS) {
		instancePtr->IsReady = 0;
		return status;
	}
    
    status = XUartNs550_SetFifoThreshold(instancePtr, 
        uartCfgTable[deviceNum].triggerLevel);
    
    if (status != XST_SUCCESS) {
		instancePtr->IsReady = 0;
		return status;
    }
    
    /* 清除历史状态 */
    XUartNs550_ClearStats(instancePtr);
    
    return XST_SUCCESS;
}

/*****************************************************************************
 * 函数名称: uint32_t UartNs550Send(uint16_t deviceNum, uint8_t *bufferPtr,uint32_t numBytes)
 * 函数说明: 串口发送
 * 输入参数:
 *        deviceNum：设备号
 *        bufferPtr：发送缓冲指针
 *        numBytes：发送数据长度，单位字节
 * 输出参数: 无
 * 返 回 值: 调用函数时发送的数据长度。
 * 备注：中断模式，非阻塞式发送
*****************************************************************************/
uint32_t UartNs550Send(uint16_t deviceNum, uint8_t *bufferPtr,uint32_t numBytes)
{
    
    XUartNs550 * instancePtr = &(uartCfgTable[deviceNum].instance);
    
    return XUartNs550_Send(instancePtr, bufferPtr, numBytes);
}

/*****************************************************************************
 * 函数名称: uint32_t UartNs550Recv(uint16_t deviceNum, uint8_t *bufferPtr,uint32_t numBytes)
 * 函数说明: 串口接收
 * 输入参数:
 *        deviceNum：设备号
 *        bufferPtr：接收缓冲指针
 *        numBytes：接收数据长度，单位字节
 * 输出参数: 无
 * 返 回 值: 调用函数时接收的数据长度
 * 备注：中断模式，非阻塞式接收
*****************************************************************************/
uint32_t UartNs550Recv(uint16_t deviceNum, uint8_t *bufferPtr,uint32_t numBytes)
{
    
    XUartNs550 * instancePtr = &(uartCfgTable[deviceNum].instance);
    
    return XUartNs550_Recv(instancePtr, bufferPtr, numBytes);
}

/*****************************************************************************
 * 函数名称: void UartNs550IntrHandler(uint16_t deviceNum)
 * 函数说明: 串口中断处理函数
 * 输入参数:
 *        deviceNum：设备号
 * 输出参数: 无
 * 返 回 值: 无
 * 备注：
*****************************************************************************/
void UartNs550IntrHandler(uint16_t deviceNum)
{
    XUartNs550 * instancePtr = &(uartCfgTable[deviceNum].instance);
    XUartNs550_InterruptHandler(instancePtr);
}

/*****************************************************************************
 * 函数名称: void UartNs550HardIntMask(uint16_t deviceNum)
 * 函数说明: 串口硬件中断Mask
 * 输入参数:
 *        deviceNum：设备号
 * 输出参数: 无
 * 返 回 值: 无
 * 备注：被Mask的设备才能有效上报中断
*****************************************************************************/
void UartNs550HardIntMask(uint16_t deviceNum)
{
    uint8_t Reg;
    Reg = *(volatile uint16_t *) (UART_INT_MASK_ADDR);
    *(volatile uint16_t *) (UART_INT_MASK_ADDR) =  Reg | (1 << deviceNum);
}

/*****************************************************************************
 * 函数名称: void UartNs550HardIntUnmask(uint16_t deviceNum)
 * 函数说明: 串口硬件中断Unmask
 * 输入参数:
 *        deviceNum：设备号
 * 输出参数: 无
 * 返 回 值: 无
 * 备注：取消Mask的设备无法有效上报中断
*****************************************************************************/
void UartNs550HardIntUnmask(uint16_t deviceNum)
{
    uint8_t Reg;
    Reg = *(volatile uint16_t *) (UART_INT_MASK_ADDR);
    *(volatile uint16_t *) (UART_INT_MASK_ADDR) =  Reg & (~(1 << deviceNum));
}

/*****************************************************************************
 * 函数名称: UartNs550GetLastErrors(uint16_t deviceNum)
 * 函数说明: 串口错误统计
 * 输入参数:
 *        deviceNum：设备号
 * 输出参数: 无
 * 返 回 值: 无
 * 备注：
*****************************************************************************/
uint8_t UartNs550GetLastErrors(uint16_t deviceNum)
{
    XUartNs550 * instancePtr = &(uartCfgTable[deviceNum].instance);
    
    return XUartNs550_GetLastErrors(instancePtr);
}

void UartNs550SetMode(uint16_t deviceNum,uint8_t mode)
{
    uint8_t Reg;
    Reg = *(volatile uint16_t *) (UART_RS485_ADDR);
    if(UART_RS485_MODE == mode)
        *(volatile uint16_t *) (UART_RS485_ADDR) =  Reg | (1 << deviceNum);
    else
        *(volatile uint16_t *) (UART_RS485_ADDR) =  Reg & (~(1 << deviceNum));
}   
    
void UartNs550RS485TxDisable(uint16_t deviceNum)
{
    uint8_t mcrRegister;
    XUartNs550 * instancePtr = &(uartCfgTable[deviceNum].instance);

    mcrRegister = XUartNs550_ReadReg(instancePtr->BaseAddress,
						XUN_MCR_OFFSET);
    
    XUartNs550_WriteReg(instancePtr->BaseAddress, XUN_MCR_OFFSET,
			 	mcrRegister | XUN_MCR_RTS);
}

void UartNs550RS485TxEnable(uint16_t deviceNum)
{
    uint8_t mcrRegister;
    XUartNs550 * instancePtr = &(uartCfgTable[deviceNum].instance);

    mcrRegister = XUartNs550_ReadReg(instancePtr->BaseAddress,
						XUN_MCR_OFFSET);
    
    XUartNs550_WriteReg(instancePtr->BaseAddress, XUN_MCR_OFFSET,
			 	mcrRegister &(~XUN_MCR_RTS) );
}

