#include "mpu9250_drv.h"
/*SYSBIOS includes*/
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>

extern void vOutputString( const char * const pcMessage,int numBytesToWrite);
mpu9250Data_t mpu9250Data;

static void mpu9250Task(void)
{
    uint8_t state;
    uint8_t str[] = "MPU9250 not found!\r\n";
    state = mpu9250Init();
    if(MPU_STATUS_ERROR ==  state)
        vOutputString(str,strlen(str));
    else;
    
    while(1)
    {
        Task_sleep(50);
        mpu9250Read(&mpu9250Data);
    }
}

void testMPU9250TaskInit(void)
{
    Task_Handle task;
	Task_Params taskParams;
    Task_Params_init(&taskParams);
    
	taskParams.priority = 5;
	taskParams.stackSize = 2048;
    
	task = Task_create(mpu9250Task, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}

