#ifndef __UARTNS550_H__
#define __UARTNS550_H__

#include "Xuartns550.h"
#include "Xparameters.h"

//#define UART_DEVICE_NUMS XPAR_XUARTNS550_NUM_INSTANCES

#define NELEMENTS(array)		/* number of elements in an array */ \
		(sizeof (array) / sizeof ((array) [0]))
		
#define BUFFER_MAX_SIZE 32
#define BUFFER_MAX_DEPTH 32

#define UART_RESET_ADDR (SOC_EMIFA_CS2_ADDR + (0x22<<1))
#define UART_INT_ENABLE_ADDR (SOC_EMIFA_CS2_ADDR + (0x24<<1))
#define UART_INT_MASK_ADDR (SOC_EMIFA_CS2_ADDR + (0x25<<1))


#define UartNs550HardReset(DeviceNum) do {\
    /* 串口控制器硬件复位 */ \
        *(volatile u16 *) (UART_RESET_ADDR) = (1 << DeviceNum); \
        *(volatile u16 *) (UART_RESET_ADDR) = 0; \
    }while(0)


#define UartNs550HardIntEnable() \
    (*(volatile u16 *) (UART_INT_ENABLE_ADDR) = 1)
    
#define UartNs550HardIntDisable() \
    (*(volatile u16 *) (UART_INT_ENABLE_ADDR) = 0)

#define UartNs550HardIntMaskAll() \
    (*(volatile u16 *) (UART_INT_MASK_ADDR) = 0xff)
    
#define UartNs550HardIntUnMaskAll() \
    (*(volatile u16 *) (UART_INT_MASK_ADDR) = 0)

typedef struct HW_UART_BUFFER
{
    u8                      Buffer[BUFFER_MAX_SIZE];
    u8                      Length;
}UART550_BUFFER;

typedef struct HW_UART_LOOP_BUFFER
{
    UART550_BUFFER          LoopBuffer[BUFFER_MAX_DEPTH];
    u8                      WritePoint;
    u8                      ReadPoint;
}UART550_LOOP_BUFFER;


typedef struct HW_UART_CFG 
{
    u16                     DeviceNum;		/**< Unique Num  of device */
	u32                     BaseAddress;	/**< Base address of device */
	u32                     InputClockHz;	/**< Input clock frequency */
    XUartNs550Format        DataFormat;     /**< Data Format */
    u16                     Options;        /**< Options */
    u8                      TriggerLevel;   /**< FIFO TriggerLevel */
    XUartNs550              Instance;       /**Ns550 Instance */
    UART550_LOOP_BUFFER     Buffer;         /**Loop Buffer */
}UART_CFG_TABLE;




s32 UartNs550Init(u16 DeivceNum,XUartNs550_Handler FuncPtr);
u32 UartNs550Send(u16 DeivceNum, u8 *BufferPtr,u32 NumBytes);
u32 UartNs550Recv(u16 DeivceNum, u8 *BufferPtr,u32 NumBytes);
void UartNs550IntrHandler(u16 DeivceNum);
void UartNs550HardIntMask(u16 DeviceNum);
void UartNs550HardIntUnmask(u16 DeviceNum);
void UartNs550InitBuffer(u16 DeviceNum);
u8 * UartNs550PushBuffer(u16 DeviceNum,u8 Length);
UART550_BUFFER* UartNs550PopBuffer(u16 DeviceNum);
u8 UartNs550GetLastErrors(u16 DeviceNum);
u8 UartNs550BufferIsEmpty(u16 DeviceNum);
#endif




