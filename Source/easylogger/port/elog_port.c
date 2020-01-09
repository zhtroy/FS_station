/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for RT-Thread.
 * Created on: 2015-04-28
 */

#include <elog.h>
#include <elog_flash.h>
#include <stdio.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/System.h>
#include "common.h"
static Semaphore_Handle output_lock;

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
static Semaphore_Handle output_notice;

static void async_output(void *arg);
#endif

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_COUNTING;
    semParams.instance->name = "elog lock";
    output_lock = Semaphore_create(1, &semParams, NULL);

    //rt_sem_init(&output_lock, "elog lock", 1, RT_IPC_FLAG_PRIO);
    
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    Task_Handle async_thread = NULL;
    Error_Block eb;
    Task_Params taskParams;
    
    semParams.instance->name = "output notice";
    output_notice = Semaphore_create(0, &semParams, NULL);

    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    taskParams.stackSize = 2048;
    taskParams.instance->name = "async_output";

    async_thread = Task_create(async_output, &taskParams, &eb);

    if (async_thread == NULL) {
        System_printf("Task async_thread create failed!\n");
        BIOS_exit(0);
    }
#endif

    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* output to terminal */
    //rt_kprintf("%.*s", size, log);
    //UARTprintf("%.*s", size, log);
    sb_puts(log,size);
    /* output to flash */
    elog_flash_write(log, size);
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    //rt_sem_take(&output_lock, RT_WAITING_FOREVER);
    Semaphore_pend(output_lock, BIOS_WAIT_FOREVER);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    Semaphore_post(output_lock);
}

static int32_t rt_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    int32_t n;
    va_list args;

    va_start(args, fmt);
    n = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}
/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    static char cur_system_time[16] = { 0 };
    rt_snprintf(cur_system_time, 16, "tick:%010d", Clock_getTicks());
    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {

    return Task_Handle_name(Task_self());
}

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
void elog_async_output_notice(void) {
    Semaphore_post(output_notice);
}

static void async_output(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];

    fdOpenSession(Task_self());
    while(true) {
        /* waiting log */
        Semaphore_pend(output_notice, BIOS_WAIT_FOREVER);
        /* polling gets and outputs the log */
        while(true) {

#ifdef ELOG_ASYNC_LINE_OUTPUT
            get_log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
            get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif

            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
    fdCloseSession(Task_self());
}
#endif
