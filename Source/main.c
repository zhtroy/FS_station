/*
 *  ======== main.c ========
 */

#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Cache.h>

#include "gpio_app.h"
#include "uartStdio.h"
#include "emifa/emifa_app.h"
#include "uartns550.h"

#include "Message/Message.h"
#include "Test_.h"
#include "canModule.h"

#include "DSP_Uart/dsp_uart2.h"

#define FPGA_TEST_REG (SOC_EMIFA_CS2_ADDR + (0x5<<1))

void PeriphInit()
{

    uint16_t udelay;
	//GPIO---------------------
	gpio_init();       //初始化GPIO
	gpio_fpga_rst();  //复位FPGA

	I2CPinMuxSetup(0);	//I2C0初始化
	//DSP UART----------------------
	UARTStdioInit();  //DSP调试串口1初始化
	dsp_uart2_init(); //DSP串口2设置，连接4G

	//EMIFA--------------------------
	EMIFA_init();     //初始化EMIFA

    while(1)
    {
        *(volatile uint16_t *)FPGA_TEST_REG = 0xaa;
        if(0xaa == *(volatile uint16_t *)FPGA_TEST_REG)
            break;
        for(udelay = 10000;udelay > 0;udelay --);
    }

    while(1)
    {
        *(volatile uint16_t *)FPGA_TEST_REG = 0x55;
        if(0x55 == *(volatile uint16_t *)FPGA_TEST_REG)
            break;
        for(udelay = 10000;udelay > 0;udelay --);
    }
    
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
    
    canTableInit();

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
	//7s
	uint32_t delay = 0xFFFFFFF;
	while(delay>0){
		delay--;
	}

	PeriphInit();

	SyncInit();

	ThreadInit();

	TestEntry();

    BIOS_start();    /* does not return */
    return(0);
}
