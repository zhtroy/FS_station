#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/package/package.defs.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/package/internal/Semaphore.xdc.h>
#include <ti/sysbios/knl/package/internal/Task.xdc.h>
#include <string.h>
#include "common.h"
#include "shell.h"

static void dumpTask(Task_Handle task)
{
    Task_Stat stat;

    Task_stat(task, &stat);

    sb_printf("%-16.16s 0x%08x %3d", Task_Handle_name(task), task, stat.priority);
    if (stat.mode == Task_Mode_RUNNING)          sb_printf(" running ");
    else if (stat.mode == Task_Mode_READY)       sb_printf(" ready   ");
    else if (stat.mode == Task_Mode_BLOCKED)     sb_printf(" blocked ");
    else if (stat.mode == Task_Mode_TERMINATED)  sb_printf(" terminal");
    else if (stat.mode == Task_Mode_INACTIVE)    sb_printf(" inactive");

    sb_printf(" 0x%08x 0x%08x %02d%% 0x%08x 0x%08x\n",
                stat.stack, 
                stat.stackSize,
                (stat.used*100)/stat.stackSize,
                task->arg0,
                task->arg1
//                Task_getArg0(task),
//                Task_getArg1(task)
                );
}

static void listTasks(uint8_t argc, char **argv)
{
    Task_Object * task;
    int32_t i;


    if(argc < 3)
    {
        sb_printf("%-16.16s    addr    pri   status     sp     stack size  used     arg0       arg1   \n", "Task");
        sb_printf("----------------");
        sb_printf(        " ---------- --- -------- ---------- ---------- ------ ---------- ----------\n");
        
        for (i = 0; i < Task_Object_count(); i++) {
            task = Task_Object_get(NULL, i);
            if(argc == 1 || (argc == 2 && 0 == strncmp(argv[1], Task_Handle_name(task), strlen(argv[1]))))
            {
                dumpTask(task);
            }
        }

        task = Task_Object_first();
        while (task) {
            if(argc == 1 || (argc == 2 && 0 == strncmp(argv[1], Task_Handle_name(task), strlen(argv[1]))))
            {
                dumpTask(task);
            }
            task = Task_Object_next(task);
        }
    }
    else
    {
        sb_printf("please input \"listTasks\" or \"listTasks xxxx\"\n");
    }

}
MSH_CMD_EXPORT(listTasks, list tasks status);



static void dumpMailbox(Mailbox_Handle mbox)
{
    sb_printf("%-16.16s 0x%08x %5d %5d %5d\n",
                Mailbox_Handle_name(mbox), 
                mbox, 
                Mailbox_getMsgSize(mbox),
                Mailbox_getNumFreeMsgs(mbox),
                Mailbox_getNumPendingMsgs(mbox));
}

static void listMbox(uint8_t argc, char **argv)
{
    Mailbox_Object * mbox;
    int32_t i;
    
    sb_printf("%-16.16s    addr    size  free  pending \n", "Mailbox");
    sb_printf("----------------");
    sb_printf(        " ---------- ----- ----- ------- \n");

    for (i = 0; i < Mailbox_Object_count(); i++) {
        mbox = Mailbox_Object_get(NULL, i);
        if(argc == 1 || (argc == 2 && 0 == strncmp(argv[1], Mailbox_Handle_name(mbox), strlen(argv[1]))))
            dumpMailbox(mbox);
    }

    mbox = Mailbox_Object_first();
    while (mbox) {
        if(argc == 1 || (argc == 2 && 0 == strncmp(argv[1], Mailbox_Handle_name(mbox), strlen(argv[1]))))
            dumpMailbox(mbox);
        mbox = Mailbox_Object_next(mbox);
    }
}
MSH_CMD_EXPORT(listMbox, list mailbox status);


static void dumpSemaphore(Semaphore_Handle sema)
{
    sb_printf("%-16.16s 0x%08x %5d",
                Semaphore_Handle_name(sema), 
                sema, 
                Semaphore_getCount(sema));
    Semaphore_Object *semobj;
    semobj = (Semaphore_Object *) sema;
    if(semobj->mode == Semaphore_Mode_BINARY)                 sb_printf(" binary\n");
    else if(semobj->mode == Semaphore_Mode_BINARY_PRIORITY)   sb_printf(" binary_pri\n");
    else if(semobj->mode == Semaphore_Mode_COUNTING)          sb_printf(" count\n");
    else if(semobj->mode == Semaphore_Mode_COUNTING_PRIORITY) sb_printf(" count_pri\n");
}

static void listSema(uint8_t argc, char **argv)
{
    Semaphore_Object * sema;
    int32_t i;
    
    sb_printf("%-16.16s    addr    count    mode    \n", "Semaphore");
    sb_printf("----------------");
    sb_printf(        " ---------- ----- ---------- \n");

    for (i = 0; i < Semaphore_Object_count(); i++) {
        sema = Semaphore_Object_get(NULL, i);
        if(argc == 1 || (argc == 2 && 0 == strncmp(argv[1], Semaphore_Handle_name(sema), strlen(argv[1]))))
            dumpSemaphore(sema);
    }

    sema = Semaphore_Object_first();
    while (sema) {
        if(argc == 1 || (argc == 2 && 0 == strncmp(argv[1], Semaphore_Handle_name(sema), strlen(argv[1]))))
            dumpSemaphore(sema);
        sema = Semaphore_Object_next(sema);
    }
}
MSH_CMD_EXPORT(listSema, list semaphore status);

