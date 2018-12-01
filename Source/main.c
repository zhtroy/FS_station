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

/*
 *  ======== taskFxn ========
 */
Void taskFxn(UArg a0, UArg a1)
{
    System_printf("enter taskFxn()\n");
    while(1){
		gpio_toggle_led();
		Task_sleep(1000);
		UARTPuts("Tronlong SYS/BIOS UART1 Application......\r\n", -1);
    }

    System_printf("exit taskFxn()\n");
}


void PeriphInit()
{
	gpio_init();
	gpio_fpga_rst();  //复位FPGA

	UARTStdioInit();  //DSP调试串口1初始化

	EMIFA_init();     //初始化EMIFA
}

void ThreadInit()
{
	//task
	Task_Handle task;
	Error_Block eb;

	System_printf("enter main()\n");
	System_flush();
	Error_init(&eb);
	task = Task_create(taskFxn, NULL, &eb);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}
/*
 *  ======== main ========
 */
Int main()
{
	PeriphInit();

	ThreadInit();


    BIOS_start();    /* does not return */
    return(0);
}
