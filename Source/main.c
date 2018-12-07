/*
 *  ======== main.c ========
 */

#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>

#include "gpio_app.h"
#include "uartStdio.h"
#include "emifa/emifa_app.h"
#include "uartns550.h"

#include "Message/Message.h"
#include "Test_.h"
#include "canModule.h"


void PeriphInit()
{

	//GPIO---------------------
	gpio_init();       //初始化GPIO
	gpio_fpga_rst();  //复位FPGA

	//DSP UART----------------------
	UARTStdioInit();  //DSP调试串口1初始化

	//EMIFA--------------------------
	EMIFA_init();     //初始化EMIFA

	//FPGA UART--------------------------

    // 串口0硬件中断MASK
//    UartNs550HardIntMask (UART0_DEVICE);

    // MASK所有串口硬件中断；
    UartNs550HardIntMaskAll();

    // 使能串口硬件中断
    UartNs550HardIntEnable ();

    // MASK所有串口硬件中断；
    canHardIntMaskAll();

    // 使能串口硬件中断
    canHardIntEnable ();


}

void ThreadInit()
{
	//task==========================
}

void SyncInit()
{
	//消息队列初始化
	Message_init();
}

/*
 *  ======== main ========
 */
Int main()
{
	PeriphInit();

	SyncInit();

	ThreadInit();

	TestEntry();

    BIOS_start();    /* does not return */
    return(0);
}
