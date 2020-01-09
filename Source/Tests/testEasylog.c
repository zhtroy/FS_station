/*
 * testEasylog.c
 *
 *  Created on: 2019-12-17
 *      Author: DELL
 */
#define LOG_TAG    "testEasylog"
#define LOG_LVL     ELOG_LVL_INFO
#include <elog.h>
#include <easyflash.h>
#include <elog_flash.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include "shell.h"


/* EasyLogger断言钩子方法 */
static void elog_user_assert_hook(const char* ex, const char* func, size_t line) {
    /* 失能异步输出方式（异步输出模块自带方法） */
    elog_async_enabled(false);
    /* 失能日志输出锁 */
    elog_output_lock_enabled(false);
    /* 失能 EasyLogger 的 Flash 插件自带同步锁（Flash 插件自带方法） */
    elog_flash_lock_enabled(false);
    /* 输出断言信息 */
    elog_a("elog", "(%s) has assert failed at %s:%ld.\n", ex, func, line);
    /* 将缓冲区中所有日志保存至 Flash （Flash 插件自带方法） */
    elog_flash_flush();
    while(1);
}
static void easylogInit(UArg arg0, UArg arg1)
{
    if ((easyflash_init() == EF_NO_ERR)&&(elog_init() == ELOG_NO_ERR)) {

        /* set EasyLogger log format */
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME |ELOG_FMT_T_INFO);
        elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME |ELOG_FMT_T_INFO);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);

        elog_assert_set_hook(elog_user_assert_hook);
        elog_flash_init();
        elog_start();
    }
    else
    {
        sb_puts("easyflash or elog initial failed\r\n",-1);
    }

}

void testEasylogTask(void)
{
    Task_Handle task;
    Error_Block eb;
    Task_Params taskParams;

    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;
    taskParams.instance->name = "easylogtask";
    task = Task_create(easylogInit, &taskParams, &eb);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

}
