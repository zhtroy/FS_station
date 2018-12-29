/***************************** Include Files ********************************/
#include "UartNs550.h"

/************************** Constant Definitions ****************************/

UART_CFG_TABLE uart_cfg_table[] = {

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
#define UART_CFG_NUM (NELEMENTS(uart_cfg_table)-1)

s32 UartNs550Init(u16 DeviceNum,XUartNs550_Handler FuncPtr)
{
    s32 Status;
    XUartNs550 * InstancePtr;
    
    if(DeviceNum > UART_CFG_NUM)
    {
        return XST_FAILURE;
    }
    
    InstancePtr = &(uart_cfg_table[DeviceNum].Instance);
    
    /*
	 * Hardware Reset UART Controller.
	 */

    UartNs550HardReset(DeviceNum);
    
    UartNs550InitBuffer(DeviceNum);
    /*
	 * Setup the data that is from the configuration information
	 */
	 
	InstancePtr->BaseAddress = uart_cfg_table[DeviceNum].BaseAddress;
	InstancePtr->InputClockHz = uart_cfg_table[DeviceNum].InputClockHz;
    //InstancePtr->BaudRate = uart_cfg_table[DeviceNum].BaudRate;

	/*
	 * Initialize the instance data to some default values and setup
	 * a handler and CallBackRef.
 	 */
 	
	InstancePtr->Handler = FuncPtr;
    //InstancePtr->CallBackRef = InstancePtr;
    InstancePtr->CallBackRef = &(uart_cfg_table[DeviceNum].DeviceNum);
    
	InstancePtr->SendBuffer.NextBytePtr = NULL;
	InstancePtr->SendBuffer.RemainingBytes = 0;
	InstancePtr->SendBuffer.RequestedBytes = 0;
	InstancePtr->ReceiveBuffer.NextBytePtr = NULL;
	InstancePtr->ReceiveBuffer.RemainingBytes = 0;
	InstancePtr->ReceiveBuffer.RequestedBytes = 0;

    /*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    Status = XUartNs550_SetDataFormat(InstancePtr, 
        &(uart_cfg_table[DeviceNum].DataFormat));
    
	if (Status != XST_SUCCESS) {
		InstancePtr->IsReady = 0;
		return Status;
	}

    Status = XUartNs550_SetOptions(InstancePtr, 
        uart_cfg_table[DeviceNum].Options);
    
	if (Status != XST_SUCCESS) {
		InstancePtr->IsReady = 0;
		return Status;
	}
    
    Status = XUartNs550_SetFifoThreshold(InstancePtr, 
        uart_cfg_table[DeviceNum].TriggerLevel);
    
    if (Status != XST_SUCCESS) {
		InstancePtr->IsReady = 0;
		return Status;
    }
    
    XUartNs550_ClearStats(InstancePtr);
    
    UartNs550Recv(DeviceNum,uart_cfg_table[DeviceNum].Buffer.LoopBuffer[0].Buffer,BUFFER_MAX_SIZE);
    
    return XST_SUCCESS;
}

u32 UartNs550Send(u16 DeviceNum, u8 *BufferPtr,u32 NumBytes)
{
    
    XUartNs550 * InstancePtr = &(uart_cfg_table[DeviceNum].Instance);
    
    return XUartNs550_Send(InstancePtr, BufferPtr, NumBytes);
}

u32 UartNs550Recv(u16 DeviceNum, u8 *BufferPtr,u32 NumBytes)
{
    
    XUartNs550 * InstancePtr = &(uart_cfg_table[DeviceNum].Instance);
    
    return XUartNs550_Recv(InstancePtr, BufferPtr, NumBytes);
}

void UartNs550InitBuffer(u16 DeviceNum)
{
    UART550_LOOP_BUFFER *LoopBuf = &(uart_cfg_table[DeviceNum].Buffer);
    
    LoopBuf->ReadPoint = 0;
    
    LoopBuf->WritePoint = 0;
}
u8 UartNs550BufferIsEmpty(u16 DeviceNum)
{
    UART550_LOOP_BUFFER *LoopBuf = &(uart_cfg_table[DeviceNum].Buffer);
    
    return (LoopBuf->ReadPoint == LoopBuf->WritePoint) ? 1 :0; 
}

UART550_BUFFER* UartNs550PopBuffer(u16 DeviceNum)
{
    UART550_LOOP_BUFFER *LoopBuf = &(uart_cfg_table[DeviceNum].Buffer);
    
    UART550_BUFFER * Buffer = &(LoopBuf->LoopBuffer[LoopBuf->ReadPoint]);
    
    LoopBuf->ReadPoint = (LoopBuf->ReadPoint+1)%BUFFER_MAX_DEPTH;
    return Buffer;
}

u8 * UartNs550PushBuffer(u16 DeviceNum,u8 Length)
{
    UART550_LOOP_BUFFER *LoopBuf = &(uart_cfg_table[DeviceNum].Buffer);

    LoopBuf->LoopBuffer[LoopBuf->WritePoint].Length = Length;
    
    LoopBuf->WritePoint = (LoopBuf->WritePoint+1)%BUFFER_MAX_DEPTH;

    /*初始化数据Buffer数据长度信息*/
    LoopBuf->LoopBuffer[LoopBuf->WritePoint].Length = 0;
    
    /*返回数据Buffer指针*/
    return LoopBuf->LoopBuffer[LoopBuf->WritePoint].Buffer;
}

void UartNs550IntrHandler(u16 DeviceNum)
{
    XUartNs550 * InstancePtr = &(uart_cfg_table[DeviceNum].Instance);
    XUartNs550_InterruptHandler(InstancePtr);
}



void UartNs550HardIntMask(u16 DeviceNum)
{
    u8 Reg;
    Reg = *(volatile u16 *) (UART_INT_MASK_ADDR);
    *(volatile u16 *) (UART_INT_MASK_ADDR) =  Reg | (1 << DeviceNum);
}
    
void UartNs550HardIntUnmask(u16 DeviceNum)
{
    u8 Reg;
    Reg = *(volatile u16 *) (UART_INT_MASK_ADDR);
    *(volatile u16 *) (UART_INT_MASK_ADDR) =  Reg & (~(1 << DeviceNum));
}

u8 UartNs550GetLastErrors(u16 DeviceNum)
{
    XUartNs550 * InstancePtr = &(uart_cfg_table[DeviceNum].Instance);
    
    return XUartNs550_GetLastErrors(InstancePtr);
}

void UartNs550SetMode(u16 DeviceNum,u8 mode)
{
    u8 Reg;
    Reg = *(volatile u16 *) (UART_RS485_ADDR);
    if(UART_RS485_MODE == mode)
        *(volatile u16 *) (UART_RS485_ADDR) =  Reg | (1 << DeviceNum);
    else
        *(volatile u16 *) (UART_RS485_ADDR) =  Reg & (~(1 << DeviceNum));
}   
    
void UartNs550RS485TxDisable(u16 DeviceNum)
{
    u8 mcrRegister;
    XUartNs550 * InstancePtr = &(uart_cfg_table[DeviceNum].Instance);

    mcrRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_MCR_OFFSET);
    
    XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_MCR_OFFSET,
			 	mcrRegister | XUN_MCR_RTS);
}

void UartNs550RS485TxEnable(u16 DeviceNum)
{
    u8 mcrRegister;
    XUartNs550 * InstancePtr = &(uart_cfg_table[DeviceNum].Instance);

    mcrRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_MCR_OFFSET);
    
    XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_MCR_OFFSET,
			 	mcrRegister &(~XUN_MCR_RTS) );
}

