
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "common.h"
#include "Version.h"
#include <xdc/runtime/Memory.h>

extern void timerWatchDogInit();


static int getVersion(uint8_t argc, uint8_t **argv)
{
    int ret = 0;
    char star = (GIT_CLEAN==0)?'*':' ';
	sb_printf("[version:%x%c]\r\n",BUILD_NUMBER,star);
    return ret;
}
MSH_CMD_EXPORT(getVersion, get version);


static int reset(uint8_t argc, uint8_t **argv)
{
    int ret = 0;
    timerWatchDogInit();
    sb_printf("CPU should reset within 1s ...\r\n");
    return ret;
}
MSH_CMD_EXPORT(reset,reset cpu);

static int mem(uint8_t argc, uint8_t **argv)
{
    int ret = 0;
	Memory_Stats status;
	Memory_getStats(NULL, &status);
	sb_printf("memory usage:\t total:%d\tfree:%d\tlargest free:%d\r\n"
			,status.totalSize
			,status.totalFreeSize
			,status.largestFreeSize);
    return ret;
}
MSH_CMD_EXPORT(mem,show alloc() memory usage);


