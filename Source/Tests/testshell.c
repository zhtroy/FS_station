/*
 * testshell.c
 *
 *  Created on: 2019-12-25
 *      Author: DELL
 */

#include "shell.h"
#include "uartStdio.h"
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>

static SHELL_TypeDef shell;
static Task_Handle taskhanle_shell;
extern uint8_t UARTSemGetc(uint8_t * val,uint32_t timeout);
//extern const unsigned int _shell_command_start = 0xc0000000;
//extern const unsigned int _shell_command_end = 0xc0000000;
static char getChar(char *c)
{
    UARTSemGetc(c,BIOS_WAIT_FOREVER);
    return 0;
}

void shell_createTask()
{
    Error_Block eb;
    Task_Params taskParams;
    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;
    taskParams.instance->name = "shellTask";
    taskParams.arg0 = (xdc_UArg)(&shell);

    taskhanle_shell = Task_create(shellTask, &taskParams, &eb);
    if (taskhanle_shell == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }
}

void shell_defaultRegister()
{
}

void testShellTask(void)
{
    Task_Handle task;
    Error_Block eb;
    Task_Params taskParams;

    shell.read = getChar;
    shell.write = UARTPutc;

    shellInit(&shell);
    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;
    taskParams.instance->name = "shellTask";
    taskParams.arg0 = (xdc_UArg)(&shell);

    task = Task_create(shellTask, &taskParams, &eb);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

}
